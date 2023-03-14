cmd_drivers/framework/task/modules.order := {  :; } | awk '!x[$$0]++' - > drivers/framework/task/modules.order
