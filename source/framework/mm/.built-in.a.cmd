cmd_drivers/framework/mm/built-in.a := rm -f drivers/framework/mm/built-in.a;  printf "drivers/framework/mm/%s " page/built-in.a | xargs /home/rlk/xiafeng.luo/CrossCompileTools/gcc-linaro-12.2.1-2022.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-ar cDPrST drivers/framework/mm/built-in.a