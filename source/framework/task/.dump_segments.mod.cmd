cmd_drivers/framework/task/dump_segments.mod := printf '%s\n'   dump_segments.o | awk '!x[$$0]++ { print("drivers/framework/task/"$$0) }' > drivers/framework/task/dump_segments.mod
