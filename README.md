# ati_ft_sensor_soem
Ethercat Master using SOEM for ATI F/T Sensor

Usage After Cloning:

```
git submodule init

sudo apt install cmake \
  build-essential \
  build-ninja

cmake --preset default
cmake --build --preset default
```

The sample binaries needs access to raw sockets to function properly. Run the following command to give the binary these capabilities (example for ec_sample):

```
sudo setcap cap_net_raw+ep build/default/bin/ec_sample
```
