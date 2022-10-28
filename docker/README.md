# IPFIXcol2 as docker

This dockerized version of [ipfixcol2](https://github.com/CESNET/ipfixcol2/issues/69) prints decoded IPFIX-message directly on stdout. 

## Usage

```bash
docker compose up
```

After usage:
```bash
docker compose down
```
### Change default config 

IPFIXcol2 supports many more output plugins such as Kafka, json or file-storage. Adapt and add output plugins to ./config/ipfixcol2.xml

Reload config file via
```
sudo docker-compose up --build --remove-orphans --force-recreate
```


