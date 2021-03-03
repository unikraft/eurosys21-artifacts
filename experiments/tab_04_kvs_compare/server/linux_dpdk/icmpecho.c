/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2013 6WIND S.A.
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include <sys/queue.h>
#include <sys/stat.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_cycles.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_memory.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_arp.h>
#include <rte_ip.h>
#include <rte_icmp.h>
#include <rte_string_fns.h>
#include <rte_flow.h>
#include <rte_udp.h>

#include "testpmd.h"


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	/* OK */

#define INITIAL_SIZE (256)
#define MAX_CHAIN_LENGTH (8)

typedef void *any_t;
/* We need to keep keys and values */
typedef struct _hashmap_element{
	char* key;
	int in_use;
	any_t data;
} hashmap_element;

/* A hashmap has some maximum size and current size,
 *  * as well as the data to hold. */
typedef struct _hashmap_map{
	int table_size;
	int size;
	hashmap_element *data;
} hashmap_map;

typedef any_t map_t;
typedef int (*PFany)(any_t, any_t);


map_t hashmap_new();
void hashmap_free(map_t in);
int hashmap_put(map_t in, char* key, any_t value);
int hashmap_length(map_t in);

/*
 *  * Return an empty hashmap, or NULL on failure.
 *   */
map_t hashmap_new() {
	hashmap_map* m = (hashmap_map*) malloc(sizeof(hashmap_map));
	if(!m) goto err;

	m->data = (hashmap_element*) calloc(INITIAL_SIZE, sizeof(hashmap_element));
	if(!m->data) goto err;

	m->table_size = INITIAL_SIZE;
	m->size = 0;

	return m;
	err:
		if (m)
			hashmap_free(m);
		return NULL;
}

static unsigned long crc32_tab[] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
   };

/* Return a 32-bit CRC of the contents of the buffer. */
unsigned long crc32(const unsigned char *s, unsigned int len)
{
  unsigned int i;
  unsigned long crc32val;
  
  crc32val = 0;
  for (i = 0;  i < len;  i ++)
    {
      crc32val =
	crc32_tab[(crc32val ^ s[i]) & 0xff] ^
	  (crc32val >> 8);
    }
  return crc32val;
}

/*
 *  * Hashing function for a string
 *   */
unsigned int hashmap_hash_int(hashmap_map * m, char* keystring){

    unsigned long key = crc32((unsigned char*)(keystring), strlen(keystring));

	/* Robert Jenkins' 32 bit Mix Function */
	key += (key << 12);
	key ^= (key >> 22);
	key += (key << 4);
	key ^= (key >> 9);
	key += (key << 10);
	key ^= (key >> 2);
	key += (key << 7);
	key ^= (key >> 12);

	/* Knuth's Multiplicative Method */
	key = (key >> 3) * 2654435761;

	return key % m->table_size;
}

/*
 *  * Return the integer of the location in data
 *   * to store the point to the item, or MAP_FULL.
 *    */
int hashmap_hash(map_t in, char* key){
	int curr;
	int i;

	/* Cast the hashmap */
	hashmap_map* m = (hashmap_map *) in;

	/* If full, return immediately */
	if(m->size >= (m->table_size/2)) return MAP_FULL;

	/* Find the best index */
	curr = hashmap_hash_int(m, key);

	/* Linear probing */
	for(i = 0; i< MAX_CHAIN_LENGTH; i++){
		if(m->data[curr].in_use == 0)
			return curr;

		if(m->data[curr].in_use == 1 && (strcmp(m->data[curr].key,key)==0))
			return curr;

		curr = (curr + 1) % m->table_size;
	}

	return MAP_FULL;
}

/*
 *  * Doubles the size of the hashmap, and rehashes all the elements
 *   */
int hashmap_rehash(map_t in){
	int i;
	int old_size;
	hashmap_element* curr;

	/* Setup the new elements */
	hashmap_map *m = (hashmap_map *) in;
	hashmap_element* temp = (hashmap_element *)
		calloc(2 * m->table_size, sizeof(hashmap_element));
	if(!temp) return MAP_OMEM;

	/* Update the array */
	curr = m->data;
	m->data = temp;

	/* Update the size */
	old_size = m->table_size;
	m->table_size = 2 * m->table_size;
	m->size = 0;

	/* Rehash the elements */
	for(i = 0; i < old_size; i++){
        int status;

        if (curr[i].in_use == 0)
            continue;
            
		status = hashmap_put(m, curr[i].key, curr[i].data);
		if (status != MAP_OK)
			return status;
	}

	free(curr);

	return MAP_OK;
}

/*
 *  * Add a pointer to the hashmap with some key
 *   */
int hashmap_put(map_t in, char* key, any_t value){
	int index;
	hashmap_map* m;

	/* Cast the hashmap */
	m = (hashmap_map *) in;

	/* Find a place to put our value */
	index = hashmap_hash(in, key);
	while(index == MAP_FULL){
		if (hashmap_rehash(in) == MAP_OMEM) {
			return MAP_OMEM;
		}
		index = hashmap_hash(in, key);
	}

	/* Set the data */
	m->data[index].data = value;
	m->data[index].key = key;
	m->data[index].in_use = 1;
	m->size++; 

	return MAP_OK;
}

/*
 *  * Get your pointer out of the hashmap with a key
 *   */
int hashmap_get(map_t in, char* key, any_t *arg){
	int curr;
	int i;
	hashmap_map* m;

	/* Cast the hashmap */
	m = (hashmap_map *) in;

	/* Find data location */
	curr = hashmap_hash_int(m, key);

	/* Linear probing, if necessary */
	for(i = 0; i<MAX_CHAIN_LENGTH; i++){

        int in_use = m->data[curr].in_use;
        if (in_use == 1){
            if (strcmp(m->data[curr].key,key)==0){
                *arg = (m->data[curr].data);
                return MAP_OK;
            }
		}

		curr = (curr + 1) % m->table_size;
	}

	*arg = NULL;

	/* Not found */
	return MAP_MISSING;
}

/*
 *  * Iterate the function parameter over each element in the hashmap.  The
 *   * additional any_t argument is passed to the function as its first
 *    * argument and the hashmap element is the second.
 *     */
int hashmap_iterate(map_t in, PFany f, any_t item) {
	int i;

	/* Cast the hashmap */
	hashmap_map* m = (hashmap_map*) in;

	/* On empty hashmap, return immediately */
	if (hashmap_length(m) <= 0)
		return MAP_MISSING;	

	/* Linear probing */
	for(i = 0; i< m->table_size; i++)
		if(m->data[i].in_use != 0) {
			any_t data = (any_t) (m->data[i].data);
			int status = f(item, data);
			if (status != MAP_OK) {
				return status;
			}
		}

    return MAP_OK;
}

/*
 *  * Remove an element with that key from the map
 *   */
int hashmap_remove(map_t in, char* key){
	int i;
	int curr;
	hashmap_map* m;

	/* Cast the hashmap */
	m = (hashmap_map *) in;

	/* Find key */
	curr = hashmap_hash_int(m, key);

	/* Linear probing, if necessary */
	for(i = 0; i<MAX_CHAIN_LENGTH; i++){

        int in_use = m->data[curr].in_use;
        if (in_use == 1){
            if (strcmp(m->data[curr].key,key)==0){
                /* Blank out the fields */
                m->data[curr].in_use = 0;
                m->data[curr].data = NULL;
                m->data[curr].key = NULL;

                /* Reduce the size */
                m->size--;
                return MAP_OK;
            }
		}
		curr = (curr + 1) % m->table_size;
	}
	/* Data not found */
	return MAP_MISSING;
}


/* Return the length of the hashmap */
int hashmap_length(map_t in){
	hashmap_map* m = (hashmap_map *) in;
	if(m != NULL) return m->size;
	else return 0;
}

/* Deallocate the hashmap */
void hashmap_free(map_t in){
	hashmap_map* m = (hashmap_map*) in;
	free(m->data);
	free(m);
}


#define KEY_MAX_LENGTH (256)
#define KEY_PREFIX ("somekey")
#define KEY_COUNT (1024 * 1024)

typedef struct data_struct_s
{
    char key_string[KEY_MAX_LENGTH];
    int number;
} data_struct_t;

map_t mymap;
char key_string[KEY_MAX_LENGTH];
data_struct_t* value;

void init_hashmap()
{
	int index;
	int error;
	mymap = hashmap_new();

	for (index=0; index<KEY_COUNT; index+=1)
	{
		/* Store the key string along side the numerical value so we can free it later */
		value = malloc(sizeof(data_struct_t));
		snprintf(value->key_string, KEY_MAX_LENGTH, "%s%d", KEY_PREFIX, index);
		value->number = index;

		error = hashmap_put(mymap, value->key_string, value);
		assert(error==MAP_OK);
	}

}

extern char keys[256][KEY_MAX_LENGTH];

void do_processing(char * t)
{
    int error;
    int index = (int)t[0];

    //error = hashmap_get(mymap, key_string, (void**)(&value));
    //
    error = hashmap_get(mymap, keys[index], (void**)(&value));
}
/*
 * int main()
 * {
 * 	init();
 * 		do_processing("bbc");
 *
 * 		}*/



static const char *
arp_op_name(uint16_t arp_op)
{
	switch (arp_op) {
	case RTE_ARP_OP_REQUEST:
		return "ARP Request";
	case RTE_ARP_OP_REPLY:
		return "ARP Reply";
	case RTE_ARP_OP_REVREQUEST:
		return "Reverse ARP Request";
	case RTE_ARP_OP_REVREPLY:
		return "Reverse ARP Reply";
	case RTE_ARP_OP_INVREQUEST:
		return "Peer Identify Request";
	case RTE_ARP_OP_INVREPLY:
		return "Peer Identify Reply";
	default:
		break;
	}
	return "Unkwown ARP op";
}

static const char *
ip_proto_name(uint16_t ip_proto)
{
	static const char * ip_proto_names[] = {
		"IP6HOPOPTS", /**< IP6 hop-by-hop options */
		"ICMP",       /**< control message protocol */
		"IGMP",       /**< group mgmt protocol */
		"GGP",        /**< gateway^2 (deprecated) */
		"IPv4",       /**< IPv4 encapsulation */

		"UNASSIGNED",
		"TCP",        /**< transport control protocol */
		"ST",         /**< Stream protocol II */
		"EGP",        /**< exterior gateway protocol */
		"PIGP",       /**< private interior gateway */

		"RCC_MON",    /**< BBN RCC Monitoring */
		"NVPII",      /**< network voice protocol*/
		"PUP",        /**< pup */
		"ARGUS",      /**< Argus */
		"EMCON",      /**< EMCON */

		"XNET",       /**< Cross Net Debugger */
		"CHAOS",      /**< Chaos*/
		"UDP",        /**< user datagram protocol */
		"MUX",        /**< Multiplexing */
		"DCN_MEAS",   /**< DCN Measurement Subsystems */

		"HMP",        /**< Host Monitoring */
		"PRM",        /**< Packet Radio Measurement */
		"XNS_IDP",    /**< xns idp */
		"TRUNK1",     /**< Trunk-1 */
		"TRUNK2",     /**< Trunk-2 */

		"LEAF1",      /**< Leaf-1 */
		"LEAF2",      /**< Leaf-2 */
		"RDP",        /**< Reliable Data */
		"IRTP",       /**< Reliable Transaction */
		"TP4",        /**< tp-4 w/ class negotiation */

		"BLT",        /**< Bulk Data Transfer */
		"NSP",        /**< Network Services */
		"INP",        /**< Merit Internodal */
		"SEP",        /**< Sequential Exchange */
		"3PC",        /**< Third Party Connect */

		"IDPR",       /**< InterDomain Policy Routing */
		"XTP",        /**< XTP */
		"DDP",        /**< Datagram Delivery */
		"CMTP",       /**< Control Message Transport */
		"TPXX",       /**< TP++ Transport */

		"ILTP",       /**< IL transport protocol */
		"IPv6_HDR",   /**< IP6 header */
		"SDRP",       /**< Source Demand Routing */
		"IPv6_RTG",   /**< IP6 routing header */
		"IPv6_FRAG",  /**< IP6 fragmentation header */

		"IDRP",       /**< InterDomain Routing*/
		"RSVP",       /**< resource reservation */
		"GRE",        /**< General Routing Encap. */
		"MHRP",       /**< Mobile Host Routing */
		"BHA",        /**< BHA */

		"ESP",        /**< IP6 Encap Sec. Payload */
		"AH",         /**< IP6 Auth Header */
		"INLSP",      /**< Integ. Net Layer Security */
		"SWIPE",      /**< IP with encryption */
		"NHRP",       /**< Next Hop Resolution */

		"UNASSIGNED",
		"UNASSIGNED",
		"UNASSIGNED",
		"ICMPv6",     /**< ICMP6 */
		"IPv6NONEXT", /**< IP6 no next header */

		"Ipv6DSTOPTS",/**< IP6 destination option */
		"AHIP",       /**< any host internal protocol */
		"CFTP",       /**< CFTP */
		"HELLO",      /**< "hello" routing protocol */
		"SATEXPAK",   /**< SATNET/Backroom EXPAK */

		"KRYPTOLAN",  /**< Kryptolan */
		"RVD",        /**< Remote Virtual Disk */
		"IPPC",       /**< Pluribus Packet Core */
		"ADFS",       /**< Any distributed FS */
		"SATMON",     /**< Satnet Monitoring */

		"VISA",       /**< VISA Protocol */
		"IPCV",       /**< Packet Core Utility */
		"CPNX",       /**< Comp. Prot. Net. Executive */
		"CPHB",       /**< Comp. Prot. HeartBeat */
		"WSN",        /**< Wang Span Network */

		"PVP",        /**< Packet Video Protocol */
		"BRSATMON",   /**< BackRoom SATNET Monitoring */
		"ND",         /**< Sun net disk proto (temp.) */
		"WBMON",      /**< WIDEBAND Monitoring */
		"WBEXPAK",    /**< WIDEBAND EXPAK */

		"EON",        /**< ISO cnlp */
		"VMTP",       /**< VMTP */
		"SVMTP",      /**< Secure VMTP */
		"VINES",      /**< Banyon VINES */
		"TTP",        /**< TTP */

		"IGP",        /**< NSFNET-IGP */
		"DGP",        /**< dissimilar gateway prot. */
		"TCF",        /**< TCF */
		"IGRP",       /**< Cisco/GXS IGRP */
		"OSPFIGP",    /**< OSPFIGP */

		"SRPC",       /**< Strite RPC protocol */
		"LARP",       /**< Locus Address Resolution */
		"MTP",        /**< Multicast Transport */
		"AX25",       /**< AX.25 Frames */
		"4IN4",       /**< IP encapsulated in IP */

		"MICP",       /**< Mobile Int.ing control */
		"SCCSP",      /**< Semaphore Comm. security */
		"ETHERIP",    /**< Ethernet IP encapsulation */
		"ENCAP",      /**< encapsulation header */
		"AES",        /**< any private encr. scheme */

		"GMTP",       /**< GMTP */
		"IPCOMP",     /**< payload compression (IPComp) */
		"UNASSIGNED",
		"UNASSIGNED",
		"PIM",        /**< Protocol Independent Mcast */
	};

	if (ip_proto < sizeof(ip_proto_names) / sizeof(ip_proto_names[0]))
		return ip_proto_names[ip_proto];
	switch (ip_proto) {
#ifdef IPPROTO_PGM
	case IPPROTO_PGM:  /**< PGM */
		return "PGM";
#endif
	case IPPROTO_SCTP:  /**< Stream Control Transport Protocol */
		return "SCTP";
#ifdef IPPROTO_DIVERT
	case IPPROTO_DIVERT: /**< divert pseudo-protocol */
		return "DIVERT";
#endif
	case IPPROTO_RAW: /**< raw IP packet */
		return "RAW";
	default:
		break;
	}
	return "UNASSIGNED";
}

static void
ipv4_addr_to_dot(uint32_t be_ipv4_addr, char *buf)
{
	uint32_t ipv4_addr;

	ipv4_addr = rte_be_to_cpu_32(be_ipv4_addr);
	sprintf(buf, "%d.%d.%d.%d", (ipv4_addr >> 24) & 0xFF,
		(ipv4_addr >> 16) & 0xFF, (ipv4_addr >> 8) & 0xFF,
		ipv4_addr & 0xFF);
}

static void
ether_addr_dump(const char *what, const struct rte_ether_addr *ea)
{
	char buf[RTE_ETHER_ADDR_FMT_SIZE];

	rte_ether_format_addr(buf, RTE_ETHER_ADDR_FMT_SIZE, ea);
	if (what)
		printf("%s", what);
	printf("%s", buf);
}

static void
ipv4_addr_dump(const char *what, uint32_t be_ipv4_addr)
{
	char buf[16];

	ipv4_addr_to_dot(be_ipv4_addr, buf);
	if (what)
		printf("%s", what);
	printf("%s", buf);
}

static uint16_t
ipv4_hdr_cksum(struct rte_ipv4_hdr *ip_h)
{
	uint16_t *v16_h;
	uint32_t ip_cksum;

	/*
	 * Compute the sum of successive 16-bit words of the IPv4 header,
	 * skipping the checksum field of the header.
	 */
	v16_h = (unaligned_uint16_t *) ip_h;
	ip_cksum = v16_h[0] + v16_h[1] + v16_h[2] + v16_h[3] +
		v16_h[4] + v16_h[6] + v16_h[7] + v16_h[8] + v16_h[9];

	/* reduce 32 bit checksum to 16 bits and complement it */
	ip_cksum = (ip_cksum & 0xffff) + (ip_cksum >> 16);
	ip_cksum = (ip_cksum & 0xffff) + (ip_cksum >> 16);
	ip_cksum = (~ip_cksum) & 0x0000FFFF;
	return (ip_cksum == 0) ? 0xFFFF : (uint16_t) ip_cksum;
}

#define is_multicast_ipv4_addr(ipv4_addr) \
	(((rte_be_to_cpu_32((ipv4_addr)) >> 24) & 0x000000FF) == 0xE0)

/*
 * Receive a burst of packets, lookup for ICMP echo requests, and, if any,
 * send back ICMP echo replies.
 */
static void
reply_to_echo_rqsts(struct fwd_stream *fs, int proto)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf *pkt;
	struct rte_ether_hdr *eth_h;
	struct rte_vlan_hdr *vlan_h;
	struct rte_arp_hdr  *arp_h;
	struct rte_ipv4_hdr *ip_h;
	struct rte_icmp_hdr *icmp_h = NULL;
	struct rte_udp_hdr *udp_h = NULL;
	struct rte_ether_addr eth_addr;
	uint32_t retry;
	uint32_t ip_addr;
	uint16_t nb_rx;
	uint16_t nb_tx;
	uint16_t nb_replies;
	uint16_t eth_type;
	uint16_t vlan_id;
	uint16_t arp_op;
	uint16_t arp_pro;
	uint16_t udp_port;
	uint32_t cksum;
	uint8_t  i;
	int l2_len;
#ifdef RTE_TEST_PMD_RECORD_CORE_CYCLES
	uint64_t start_tsc;
	uint64_t end_tsc;
	uint64_t core_cycles;
#endif

#ifdef RTE_TEST_PMD_RECORD_CORE_CYCLES
	start_tsc = rte_rdtsc();
#endif

	/*
	 * First, receive a burst of packets.
	 */
	nb_rx = rte_eth_rx_burst(fs->rx_port, fs->rx_queue, pkts_burst,
				 nb_pkt_per_burst);
	if (unlikely(nb_rx == 0))
		return;

#ifdef RTE_TEST_PMD_RECORD_BURST_STATS
	fs->rx_burst_stats.pkt_burst_spread[nb_rx]++;
#endif
	fs->rx_packets += nb_rx;
	nb_replies = 0;
	for (i = 0; i < nb_rx; i++) {
		if (likely(i < nb_rx - 1))
			rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[i + 1],
						       void *));
		pkt = pkts_burst[i];
		eth_h = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
		eth_type = RTE_BE_TO_CPU_16(eth_h->ether_type);
		l2_len = sizeof(struct rte_ether_hdr);
		if (verbose_level > 0) {
			printf("\nPort %d pkt-len=%u nb-segs=%u\n",
			       fs->rx_port, pkt->pkt_len, pkt->nb_segs);
			ether_addr_dump("  ETH:  src=", &eth_h->s_addr);
			ether_addr_dump(" dst=", &eth_h->d_addr);
		}
		if (eth_type == RTE_ETHER_TYPE_VLAN) {
			vlan_h = (struct rte_vlan_hdr *)
				((char *)eth_h + sizeof(struct rte_ether_hdr));
			l2_len  += sizeof(struct rte_vlan_hdr);
			eth_type = rte_be_to_cpu_16(vlan_h->eth_proto);
			if (verbose_level > 0) {
				vlan_id = rte_be_to_cpu_16(vlan_h->vlan_tci)
					& 0xFFF;
				printf(" [vlan id=%u]", vlan_id);
			}
		}
		if (verbose_level > 0) {
			printf(" type=0x%04x\n", eth_type);
		}

		/* Reply to ARP requests */
		if (eth_type == RTE_ETHER_TYPE_ARP) {
			arp_h = (struct rte_arp_hdr *) ((char *)eth_h + l2_len);
			arp_op = RTE_BE_TO_CPU_16(arp_h->arp_opcode);
			arp_pro = RTE_BE_TO_CPU_16(arp_h->arp_protocol);
			if (verbose_level > 0) {
				printf("  ARP:  hrd=%d proto=0x%04x hln=%d "
				       "pln=%d op=%u (%s)\n",
				       RTE_BE_TO_CPU_16(arp_h->arp_hardware),
				       arp_pro, arp_h->arp_hlen,
				       arp_h->arp_plen, arp_op,
				       arp_op_name(arp_op));
			}
			if ((RTE_BE_TO_CPU_16(arp_h->arp_hardware) !=
			     RTE_ARP_HRD_ETHER) ||
			    (arp_pro != RTE_ETHER_TYPE_IPV4) ||
			    (arp_h->arp_hlen != 6) ||
			    (arp_h->arp_plen != 4)
			    ) {
				rte_pktmbuf_free(pkt);
				if (verbose_level > 0)
					printf("\n");
				continue;
			}
			if (verbose_level > 0) {
				rte_ether_addr_copy(&arp_h->arp_data.arp_sha,
						&eth_addr);
				ether_addr_dump("        sha=", &eth_addr);
				ip_addr = arp_h->arp_data.arp_sip;
				ipv4_addr_dump(" sip=", ip_addr);
				printf("\n");
				rte_ether_addr_copy(&arp_h->arp_data.arp_tha,
						&eth_addr);
				ether_addr_dump("        tha=", &eth_addr);
				ip_addr = arp_h->arp_data.arp_tip;
				ipv4_addr_dump(" tip=", ip_addr);
				printf("\n");
			}
			if (arp_op != RTE_ARP_OP_REQUEST) {
				rte_pktmbuf_free(pkt);
				continue;
			}

			/*
			 * Build ARP reply.
			 */

			/* Use source MAC address as destination MAC address. */
			rte_ether_addr_copy(&eth_h->s_addr, &eth_h->d_addr);
			/* Set source MAC address with MAC address of TX port */
			rte_ether_addr_copy(&ports[fs->tx_port].eth_addr,
					&eth_h->s_addr);

			arp_h->arp_opcode = rte_cpu_to_be_16(RTE_ARP_OP_REPLY);
			rte_ether_addr_copy(&arp_h->arp_data.arp_tha,
					&eth_addr);
			rte_ether_addr_copy(&arp_h->arp_data.arp_sha,
					&arp_h->arp_data.arp_tha);
			rte_ether_addr_copy(&eth_h->s_addr,
					&arp_h->arp_data.arp_sha);

			/* Swap IP addresses in ARP payload */
			ip_addr = arp_h->arp_data.arp_sip;
			arp_h->arp_data.arp_sip = arp_h->arp_data.arp_tip;
			arp_h->arp_data.arp_tip = ip_addr;
			pkts_burst[nb_replies++] = pkt;
			continue;
		}

		if (eth_type != RTE_ETHER_TYPE_IPV4) {
			rte_pktmbuf_free(pkt);
			continue;
		}
		ip_h = (struct rte_ipv4_hdr *) ((char *)eth_h + l2_len);
		if (verbose_level > 0) {
			ipv4_addr_dump("  IPV4: src=", ip_h->src_addr);
			ipv4_addr_dump(" dst=", ip_h->dst_addr);
			printf(" proto=%d (%s)\n",
			       ip_h->next_proto_id,
			       ip_proto_name(ip_h->next_proto_id));
		}

		/*
		 * Check if packet is a ICMP echo request.
		 */
		if (proto == IPPROTO_ICMP) {
			icmp_h = (struct rte_icmp_hdr *) ((char *)ip_h +
					sizeof(struct rte_ipv4_hdr));
			if (! ((ip_h->next_proto_id == IPPROTO_ICMP) &&
						(icmp_h->icmp_type == RTE_IP_ICMP_ECHO_REQUEST) &&
						(icmp_h->icmp_code == 0))) {
				rte_pktmbuf_free(pkt);
				continue;
			}
		} else if (proto == IPPROTO_UDP) {
			udp_h = (struct rte_udp_hdr *) ((char *)ip_h +
				 sizeof(struct rte_ipv4_hdr));
			if ((ip_h->next_proto_id != IPPROTO_UDP) &&
			    (rte_be_to_cpu_16(udp_h->dst_port) != 7)) {
				rte_pktmbuf_free(pkt);
				continue;
			}
 		}

		if (proto == IPPROTO_ICMP) {
			if (verbose_level > 0)
				printf("  ICMP: echo request seq id=%d\n",
				       rte_be_to_cpu_16(icmp_h->icmp_seq_nb));
		} else if (proto == IPPROTO_UDP) {
			if (verbose_level > 0)
				printf("  UDP: echo request from port=%d\n",
				       rte_be_to_cpu_16(udp_h->src_port));
		}

		/*
		 * Prepare ICMP echo reply to be sent back.
		 * - switch ethernet source and destinations addresses,
		 * - use the request IP source address as the reply IP
		 *    destination address,
		 * - if the request IP destination address is a multicast
		 *   address:
		 *     - choose a reply IP source address different from the
		 *       request IP source address,
		 *     - re-compute the IP header checksum.
		 *   Otherwise:
		 *     - switch the request IP source and destination
		 *       addresses in the reply IP header,
		 *     - keep the IP header checksum unchanged.
		 * - set RTE_IP_ICMP_ECHO_REPLY in ICMP header.
		 * ICMP checksum is computed by assuming it is valid in the
		 * echo request and not verified.
		 */
		rte_ether_addr_copy(&eth_h->s_addr, &eth_addr);
		rte_ether_addr_copy(&eth_h->d_addr, &eth_h->s_addr);
		rte_ether_addr_copy(&eth_addr, &eth_h->d_addr);
		ip_addr = ip_h->src_addr;
		if (is_multicast_ipv4_addr(ip_h->dst_addr)) {
			uint32_t ip_src;

			ip_src = rte_be_to_cpu_32(ip_addr);
			if ((ip_src & 0x00000003) == 1)
				ip_src = (ip_src & 0xFFFFFFFC) | 0x00000002;
			else
				ip_src = (ip_src & 0xFFFFFFFC) | 0x00000001;
			ip_h->src_addr = rte_cpu_to_be_32(ip_src);
			ip_h->dst_addr = ip_addr;
			ip_h->hdr_checksum = ipv4_hdr_cksum(ip_h);
		} else {
			ip_h->src_addr = ip_h->dst_addr;
			ip_h->dst_addr = ip_addr;
		}
		if (proto == IPPROTO_ICMP) {
			icmp_h->icmp_type = RTE_IP_ICMP_ECHO_REPLY;
			cksum = ~icmp_h->icmp_cksum & 0xffff;
			cksum += ~htons(RTE_IP_ICMP_ECHO_REQUEST << 8) & 0xffff;
			cksum += htons(RTE_IP_ICMP_ECHO_REPLY << 8);
			cksum = (cksum & 0xffff) + (cksum >> 16);
			cksum = (cksum & 0xffff) + (cksum >> 16);
			icmp_h->icmp_cksum = ~cksum;
		} else if (proto == IPPROTO_UDP) {
			do_processing(((char *)ip_h +sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr)));
			udp_port = udp_h->src_port;
			udp_h->src_port = udp_h->dst_port;
			udp_h->dst_port = udp_port;
		}
		pkts_burst[nb_replies++] = pkt;
	}

	/* Send back ICMP echo replies, if any. */
	if (nb_replies > 0) {
		nb_tx = rte_eth_tx_burst(fs->tx_port, fs->tx_queue, pkts_burst,
					 nb_replies);
		/*
		 * Retry if necessary
		 */
		if (unlikely(nb_tx < nb_replies) && fs->retry_enabled) {
			retry = 0;
			while (nb_tx < nb_replies &&
					retry++ < burst_tx_retry_num) {
				rte_delay_us(burst_tx_delay_time);
				nb_tx += rte_eth_tx_burst(fs->tx_port,
						fs->tx_queue,
						&pkts_burst[nb_tx],
						nb_replies - nb_tx);
			}
		}
		fs->tx_packets += nb_tx;
#ifdef RTE_TEST_PMD_RECORD_BURST_STATS
		fs->tx_burst_stats.pkt_burst_spread[nb_tx]++;
#endif
		if (unlikely(nb_tx < nb_replies)) {
			fs->fwd_dropped += (nb_replies - nb_tx);
			do {
				rte_pktmbuf_free(pkts_burst[nb_tx]);
			} while (++nb_tx < nb_replies);
		}
	}

#ifdef RTE_TEST_PMD_RECORD_CORE_CYCLES
	end_tsc = rte_rdtsc();
	core_cycles = (end_tsc - start_tsc);
	fs->core_cycles = (uint64_t) (fs->core_cycles + core_cycles);
#endif
}


static void
reply_to_icmp_echo_rqsts(struct fwd_stream *fs)
{
	reply_to_echo_rqsts(fs, IPPROTO_ICMP);
}

static void
reply_to_udp_echo_rqsts(struct fwd_stream *fs)
{
	reply_to_echo_rqsts(fs, IPPROTO_UDP);
}

 
struct fwd_engine udp_echo_engine = {
	.fwd_mode_name  = "udpecho",
	.port_fwd_begin = NULL,
	.port_fwd_end   = NULL,
	.packet_fwd     = reply_to_udp_echo_rqsts,
};

struct fwd_engine icmp_echo_engine = {
	.fwd_mode_name  = "icmpecho",
	.port_fwd_begin = NULL,
	.port_fwd_end   = NULL,
	.packet_fwd     = reply_to_icmp_echo_rqsts,
};
