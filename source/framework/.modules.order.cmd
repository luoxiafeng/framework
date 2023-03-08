cmd_drivers/framework/modules.order := {   cat drivers/framework/model/modules.order;   cat drivers/framework/time/modules.order; :; } | awk '!x[$$0]++' - > drivers/framework/modules.order
