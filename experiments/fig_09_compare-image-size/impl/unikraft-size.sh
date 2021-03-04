WORKDIR=$(dirname $0)
IMAGESDIR=$WORKDIR/../images

HELLO_SIZE=`du --block-size=1 $IMAGESDIR/unikraft-helloworld | tail -n 1 | awk '{ print $1 }'`
echo ${HELLO_SIZE}  > results/unikraft-hello.csv

NGINX_SIZE=`du --block-size=1 $IMAGESDIR/unikraft-nginx | tail -n 1 | awk '{ print $1 }'`
echo ${NGINX_SIZE}  > results/unikraft-nginx.csv

SQLITE_SIZE=`du --block-size=1 $IMAGESDIR/unikraft-sqlite | tail -n 1 | awk '{ print $1 }'`
echo ${SQLITE_SIZE}  > results/unikraft-sqlite.csv

REDIS_SIZE=`du --block-size=1 $IMAGESDIR/unikraft-redis | tail -n 1 | awk '{ print $1 }'`
echo ${REDIS_SIZE}  > results/unikraft-redis.csv
