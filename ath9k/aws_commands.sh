cd ~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/compat-wireless-2014-05-22/drivers/net/wireless/ath/ath9k/ucsma-rate-control
git checkout test
git pull

cp ath9k/*.c ../
cp ath9k/*.h ../

cd ~/399/new_ucsma/ucsma/OpenWRT-14.07-JS9331
make package/compile

cd ~/399/new_ucsma/ucsma
./buffer_counter.sh
