WORKDIR=$(realpath $(dirname $0))
MIRAGE_IMAGE=${MIRAGE_IMAGE:-unikraft/eurosys21-artifacts-mirage:latest}
CONTAINER_NAME=mirage-build-helloworld

cleanup() {
  docker stop $CONTAINER_NAME
}

trap cleanup ERR
trap cleanup EXIT

# Ensure mirage unikernel directory has correct permissions
chown -Rf 1000:65533 $WORKDIR/mirage-helloworld

# Start the build container
docker run -t -d --rm \
  --name $CONTAINER_NAME \
  -v $WORKDIR/mirage-helloworld:/usr/src/app \
  -w /usr/src/app \
  $MIRAGE_IMAGE

# Configure and build the unikernel
docker exec $CONTAINER_NAME mirage configure -t hvt
docker exec $CONTAINER_NAME make depends
docker exec $CONTAINER_NAME make

HELLO_SIZE=`du --block-size=1 $WORKDIR/mirage-helloworld/helloworld.hvt | tail -n 1 | awk '{ print $1 }'`
echo ${HELLO_SIZE} > $WORKDIR/../results/mirage-hello.csv
