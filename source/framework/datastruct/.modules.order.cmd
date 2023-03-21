cmd_drivers/framework/datastruct/modules.order := {  :; } | awk '!x[$$0]++' - > drivers/framework/datastruct/modules.order
