#!/bin/bash
# HELP: A simple calculator app
# ICON: calculator
# GRID: Calculator

. /opt/muos/script/var/func.sh
echo app >/tmp/act_go

GOV_GO="/tmp/gov_go"
[ -e "$GOV_GO" ] && cat "$GOV_GO" >"$(GET_VAR "device" "cpu/governor")"

cd "$(GET_VAR "device" "storage/rom/mount")/MUOS/application/Calculator"
./calc
