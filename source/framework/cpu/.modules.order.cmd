cmd_drivers/framework/cpu/modules.order := {  :; } | awk '!x[$$0]++' - > drivers/framework/cpu/modules.order
