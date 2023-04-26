cmd_drivers/framework/mm/page/page_ops.mod := printf '%s\n'   page_ops.o | awk '!x[$$0]++ { print("drivers/framework/mm/page/"$$0) }' > drivers/framework/mm/page/page_ops.mod
