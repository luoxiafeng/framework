cmd_drivers/framework/mm/modules.order := {   cat drivers/framework/mm/page/modules.order; :; } | awk '!x[$$0]++' - > drivers/framework/mm/modules.order
