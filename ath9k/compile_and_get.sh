git add main.c
git commit -m "$2"
git push
cat aws_commands.sh | ssh -i new_key.pem ubuntu@nas2019.asia
scp -i new_key.pem ubuntu@nas2019.asia:~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331/staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/lib/modules/3.10.49/ath9k.ko ./ath9k_$1.ko
scp -i new_key.pem ubuntu@nas2019.asia:~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k/buffer_number.ko ./buffer_number.ko
scp -i new_key.pem ubuntu@nas2019.asia:~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k/read_buffer_number ./read_buffer_number
scp -i new_key.pem ubuntu@nas2019.asia:~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k/syscall_table.ko ./syscall_table.ko
