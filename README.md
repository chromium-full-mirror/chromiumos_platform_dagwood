# EC AIC Tester

## Firmware build

```
$ unset ZEPHYR_BASE
$ west init -m ../ec-aic-tester/ec-aic-tester
$ west update -o=--depth=1 -n
$ west build ec-aic-tester
$ west flash
```
