# ati_ft_sensor_soem
Ethercat Master using SOEM for ATI F/T Sensor

Usage After Cloning:

```
git submodule init
git submodule update --init --recursive

sudo apt update
sudo apt install -y cmake build-essential ninja-build

cmake --preset default
cmake --build --preset default

make
```

The sample binaries needs access to raw sockets to function properly. Run the following command to give the binary these capabilities (example for app):

```
sudo setcap cap_net_raw+ep ./read_vals
sudo setcap cap_net_raw+ep ./zero
```

Find the ethernet port using `ip link`, then zero using:
```
sudo ./zero <port_name>
```

After zeroing, run using:
```
sudo ./read_vals <port_name> <output_filename>
``

The ouput is stored as a csv at output_filename. To plot the csv, first set up you python virtual environment:
```
sudo apt update
sudo apt install -y python3 python3-venv python3-pip

python3 -m venv plot_env
source plot_env/bin/activate

pip install --upgrade pip
pip install matplotlib
``

In order to plot, run:
```
python3 plot.py <file_to_plot>
```
