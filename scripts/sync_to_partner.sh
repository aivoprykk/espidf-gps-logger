h=$1
if [ "$h" = "" ]; then h=10.0.111.221; fi
f="."
t=$h:Downloads/Arendus/Majasa/esp-gps-logger/espidf-gps-logger-updated-20250411/
if [ "$I" != "" ]; then
t="."
f=$h:Downloads/Arendus/Majasa/esp-gps-logger/espidf-gps-logger-updated-20250411/
fi
if [ "$DRY" != "" ]; then
a=n
fi

rsync -av$a --exclude=.ccls-cache --exclude=.vscode --exclude='*.lock' --exclude=sdkconfig --exclude='*.old' --exclude=managed_components --exclude=node_modules --exclude=build --exclude=.DS_Store "$f" "$t"
