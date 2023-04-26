cmd_drivers/framework/modules.order := {   cat drivers/framework/task/modules.order;   cat drivers/framework/mm/modules.order; :; } | awk '!x[$$0]++' - > drivers/framework/modules.order
