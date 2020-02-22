cd ~/399/new_ucsma/ucsma/ucsma-rate-control/ath9k/
git pull
cp * ../../build_dir/ath/ath9k/

cd ../kernel-module
cp * ../../build_dir/ath/ath9k/

cd ../..
./build.sh
