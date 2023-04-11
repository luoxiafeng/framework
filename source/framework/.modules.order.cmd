cmd_drivers/framework/modules.order := {   cat drivers/framework/task/modules.order; :; } | awk '!x[$$0]++' - > drivers/framework/modules.order
