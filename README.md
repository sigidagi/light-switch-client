
### Generating compilation DB

[Compilation database](https://sarcasm.github.io/notes/dev/compilation-database.html)

`ninja -C out -t compdb > compile_commands.json`


## Commissioner

Commission Meross switch and allow access to the cluster OnOff (ID: 6) for client with node ID: 60
```
chip-tool pairing ble-wifi 66 clairefi2 upinthesky 79330753 771 --paa-trust-store-path /home/sigis/.config/web-gin/paa-root-certs

chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [60], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}]}]' 66 0
```

Commission Linux switch client and bind with 'Meross' switch (server)

```
chip-tool pairing onnetwork 60 20202021
chip-tool binding write binding '[{"fabricIndex": 1, "node": 66, "endpoint": 1, "cluster": 6}]' 60 1

```
