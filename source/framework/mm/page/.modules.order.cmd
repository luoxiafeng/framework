cmd_drivers/framework/mm/page/modules.order := {   echo drivers/framework/mm/page/page_ops.ko; :; } | awk '!x[$$0]++' - > drivers/framework/mm/page/modules.order
