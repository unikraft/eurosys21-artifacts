
cd xen/apps/app-helloworld/

echo "" > xen_9p.results
echo "With 9pfs (ns)"
for i in {1..5}
do
sudo xl create -c xl_9p.conf | grep "some_long_computation" | cut -d ' ' -f  4 | tee >> xen_9p.results
sleep 15
done

echo "" > xen.results
echo "Without 9pfs (ns)"
for i in {1..5}
do
sudo xl create -c xl.conf | grep "some_long_computation" | cut -d ' ' -f  4 | tee >> xen.results
sleep 15
done

cp *.results ../../../results
