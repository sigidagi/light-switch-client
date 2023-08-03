[##](##) Build

1. Create symbolic link to `connectedhomeip` in `third_party` directory
2. `source third_party/connectedhomeip/scripts/activate.sh`
3. gn gen out
4. ninja -C out

## Commissioner

Commission Meross switch and allow access to the cluster OnOff (ID: 6) for client with node ID: 60
```
chip-tool pairing ble-wifi 66 SSID wifipassword 79330753 771 --paa-trust-store-path /path-to/paa-root-certs

chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [60], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}]}]' 66 0
```

NOTE: pairing Matter device (i.e. Meross switch) setup-pin-code and discriminator is (should be) different per device.

Commission Linux switch client and bind with 'Meross' switch (server)
```
chip-tool pairing onnetwork 60 20202021
chip-tool binding write binding '[{"fabricIndex": 1, "node": 66, "endpoint": 1, "cluster": 6}]' 60 1

```

### Generating compilation DB for vim completion

[Compilation database](https://sarcasm.github.io/notes/dev/compilation-database.html)

`ninja -C out -t compdb > compile_commands.json`


