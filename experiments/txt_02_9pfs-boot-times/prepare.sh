git clone https://github.com/unikraft/unikraft
cd unikraft && git checkout 1aca74e5dceef2e50bff0bacbeb3b3900e73e78f
cd ..
cp 0001-Boot-time-measurement.patch unikraft/
cd unikraft && git apply 0001-Boot-time-measurement.patch
mkdir tmp/
