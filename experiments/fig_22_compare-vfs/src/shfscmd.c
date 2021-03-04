#include <shfs/shfs.h>
#include <shfs/shfs_fio.h>
#include <shfs/shfs_btable.h>
#include <shfs/htable.h>
#include <shfs/hash.h>
#include <uk/blkdev.h>

/*
size_t strftimestamp_s(char *s, size_t slen, const char *fmt, uint64_t ts_sec)
{
	struct tm *tm;
	time_t *tsec = (time_t *) &ts_sec;
	tm = localtime(tsec);
	return strftime(s, slen, fmt, tm);
}
*/

size_t strftimestamp_s(char *s, size_t slen, const char *fmt, uint64_t ts_sec)
{
	s[0] = 'n';
	s[1] = '/';
	s[2] = 'a';
	s[3] = '\0';
	return 4;
}

static void uuid_unparse(const uuid_t uu, char *out)
{
	sprintf(out, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	        uu[0], uu[1], uu[2], uu[3], uu[4], uu[5], uu[6], uu[7],
	        uu[8], uu[9], uu[10], uu[11], uu[12], uu[13], uu[14], uu[15]);
}

static void hash_unparse(const hash512_t h, uint8_t hlen, char *out)
{
	uint8_t i;

	for (i = 0; i < hlen; i++)
		snprintf(out + (2*i), 3, "%02x", h[i]);
}


int shfs_ls(void)
{
	struct htable_el *el;
	struct shfs_bentry *bentry;
	struct shfs_hentry *hentry;
	char str_hash[(shfs_vol.hlen * 2) + 1];
	char str_mime[sizeof(hentry->f_attr.mime) + 1];
	char str_name[sizeof(hentry->name) + 1];
	char str_date[20];

	uk_semaphore_down(&shfs_mount_lock);
	if (!shfs_mounted)
		goto out;

	str_hash[(shfs_vol.hlen * 2)] = '\0';
	str_name[sizeof(hentry->name)] = '\0';
	str_date[0] = '\0';

	if (shfs_vol.hlen <= 32)
		printf("%-64s ", "Hash");
	else
		printf("%-128s ", "Hash");
	printf("%12s %12s %5s %5s %-24s %-16s %s\n",
	       "Loc (chk)",
	       "Size (chk)",
	       "Flags",
	       "LType",
	       "MIME",
	       "Added",
	       "Name");

	foreach_htable_el(shfs_vol.bt, el) {
		bentry = el->private;
		hentry = (struct shfs_hentry *)
			((uint8_t *) shfs_vol.htable_chunk_cache[bentry->hentry_htchunk]
			 + bentry->hentry_htoffset);
		hash_unparse(*el->h, shfs_vol.hlen, str_hash);
		strncpy(str_name, hentry->name, sizeof(str_name));
		strftimestamp_s(str_date, sizeof(str_date),
		                "%b %e, %g %H:%M", hentry->ts_creation);
		if (!SHFS_HENTRY_ISLINK(hentry))
			strncpy(str_mime, hentry->f_attr.mime, sizeof(str_mime));

		/* hash */
		if (shfs_vol.hlen <= 32)
			printf("%-64s ", str_hash);
		else
			printf("%-128s ", str_hash);

		/* loc, size */
		if (SHFS_HENTRY_ISLINK(hentry))
			printf("                          ");
		else
			printf("%12"PRIchk" %12"PRIchk" ",
			       hentry->f_attr.chunk,
			       DIV_ROUND_UP(hentry->f_attr.len + hentry->f_attr.offset, shfs_vol.chunksize));

		/* flags */
		printf("  %c%c%c ",
		       (hentry->flags & SHFS_EFLAG_LINK)    ? 'L' : '-',
		       (hentry->flags & SHFS_EFLAG_DEFAULT) ? 'D' : '-',
		       (hentry->flags & SHFS_EFLAG_HIDDEN)  ? 'H' : '-');

		/* ltype, mime */
		if (SHFS_HENTRY_ISLINK(hentry)) {
			switch (hentry->l_attr.type) {
			case SHFS_LTYPE_RAW:
				printf("%5s ", "raw");
				break;
			case SHFS_LTYPE_AUTO:
				printf("%5s ", "auto");
				break;
			default: /* SHFS_LTYPE_REDIRECT */
				printf("%5s ", "redir");
				break;
			}

			printf("%-24s ", " ");
		} else {
			printf("      ");
			printf("%-24s ", str_mime);
		}

		/* date, name */
		printf("%-16s ",
		       str_date);

		printf("%s\n",
		       str_name);
	}

 out:
	uk_semaphore_up(&shfs_mount_lock);
	return 0;
}

int shfs_info(void)
{
	unsigned int m;
	char str_uuid[37];
	char str_date[20];
	int ret = 0;

	uk_semaphore_down(&shfs_mount_lock);
	if (!shfs_mounted) {
		printf("No SHFS filesystem is mounted\n");
		ret = -1;
		goto out;
	}

	printf("SHFS version:       %2x.%02x\n",
	        SHFS_MAJOR,
	        SHFS_MINOR);
	printf("Volume name:        %s\n", shfs_vol.volname);
	uuid_unparse(shfs_vol.uuid, str_uuid);
	printf("Volume UUID:        %s\n", str_uuid);
	strftimestamp_s(str_date, sizeof(str_date),
	                "%b %e, %g %H:%M", shfs_vol.ts_creation);
	printf("Creation date:      %s\n", str_date);
	printf("Chunksize:          %"PRIu32" KiB\n",
	        shfs_vol.chunksize / 1024);
	printf("Volume size:        %"PRIchk" KiB\n",
	        CHUNKS_TO_BYTES(shfs_vol.volsize, shfs_vol.chunksize) / 1024);
	printf("Hash table:         %"PRIu32" entries in %"PRIu32" buckets\n" \
	        "                    %"PRIchk" chunks (%"PRIchk" KiB)\n" \
	        "                    %s\n",
	        shfs_vol.htable_nb_entries, shfs_vol.htable_nb_buckets,
	        shfs_vol.htable_len, (shfs_vol.htable_len * shfs_vol.chunksize) / 1024,
	        shfs_vol.htable_bak_ref ? "2nd copy enabled" : "No copy");
	printf("Entry size:         %lu Bytes (raw: %zu Bytes)\n",
	        SHFS_HENTRY_SIZE, sizeof(struct shfs_hentry));

	printf("\n");
	printf("Member stripe size: %"PRIu32" KiB\n", shfs_vol.stripesize / 1024);
	printf("Member stripe mode: %s\n", (shfs_vol.stripemode == SHFS_SM_COMBINED ?
	                                          "Combined" : "Independent" ));
	printf("Volume members:     %u device(s)\n", shfs_vol.nb_members);
	for (m = 0; m < shfs_vol.nb_members; m++) {
		uuid_unparse(shfs_vol.member[m].uuid, str_uuid);
		printf("  Member %2u:\n", m);
		printf("    Device:         blkdev%u\n", uk_blkdev_id_get(shfs_vol.member[m].bd));
		printf("    UUID:           %s\n", str_uuid);
		printf("    Block size:     %"__PRIsz"\n", uk_blkdev_ssize(shfs_vol.member[m].bd));
	}

 out:
	uk_semaphore_up(&shfs_mount_lock);
	return ret;
}
