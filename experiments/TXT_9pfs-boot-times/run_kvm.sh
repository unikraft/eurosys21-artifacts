
cd xen/apps/app-helloworld/

echo "" > kvm_9p.results
echo "With 9pfs (ns)"
for i in {1..5}
do
qemu-system-x86_64 -cpu host -enable-kvm -m 256M -nodefaults -no-acpi -display none -serial stdio -device isa-debug-exit -kernel build/app-helloworld_kvm-x86_64 -fsdev local,id=myid,path=tmp/,security_model=none -device virtio-9p-pci,fsdev=myid,mount_tag=rootfs,disable-modern=on,disable-legacy=off | grep "some_long_computation" | cut -d ' ' -f  4 | tee  >> kvm_9p.results
done

echo "" > kvm.results
echo "Without 9pfs (ns)"
for i in {1..5}
do
qemu-system-x86_64 -cpu host -enable-kvm -m 256M -nodefaults -no-acpi -display none -serial stdio -device isa-debug-exit -kernel build/app-helloworld_kvm-x86_64 | grep "some_long_computation" | cut -d ' ' -f  4 | tee  >> kvm.results
done

cp *.results ../../../results
