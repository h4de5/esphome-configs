# ESPHOME-configs

## docker setup

startup container:

    docker-compose up -d

update esphome installation:

    ./update

connect to esphome:

    ./connect

shutdown container:

    docker-compose down

create a new secrets.yaml file:

    echo 'wifi_name_home: "TheWIFI"' >> secrets.yaml
    echo 'wifi_pass_home: "ThePassword"' >> secrets.yaml
    echo 'api_password: "AnotherPassword"' >> secrets.yaml

## esphome commands

when inside the container you can do the following:

test configuration:

    esphome sonoff_4ch.yaml config

compile firmware:

    esphome sonoff_4ch.yaml compile

    esphome gosund_*.yaml sonoff_*.yaml compile

upload firmware - either when device is connected via UART or if it has been flashed with `ota:` integration already:

    esphome sonoff_4ch.yaml upload

    esphome gosund_*.yaml sonoff_*.yaml upload

or both:

    esphome sonoff_4ch.yaml run

or visit the web interface http://localhost:6152
