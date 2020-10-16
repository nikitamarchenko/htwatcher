echo "update .config"
sed -i "s/CONFIG_RTE_LIBEAL_USE_HPET=n/CONFIG_RTE_LIBEAL_USE_HPET=y/g" build/.config
