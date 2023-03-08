cmd_drivers/examples/model/test.mod := printf '%s\n'   test.o | awk '!x[$$0]++ { print("drivers/examples/model/"$$0) }' > drivers/examples/model/test.mod
