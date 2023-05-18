#### server

Go on GUI, you are gonna turn on hotspot. But doing this will move rpi to connect to its own hotspot unless you are connected to ethernet
Turn on hotspot
```
systemctl status NetworkManager
nmcli radio wifi on
sudo nmcli device wifi hotspot con-name nimp ssid nimp password sussybaka
```

Turn off hotspot
```
nmcli connection delete nimp
```