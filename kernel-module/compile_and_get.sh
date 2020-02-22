git add *
git commit -m "$2"
git push
cat aws_commands.sh | ssh -i ../ath9k/new_key.pem ubuntu@nas2019.asia
scp -i new_key.pem ubuntu@nas2019.asia:~/399/new_ucsma/ucsma/result/modules/unlock.ko ./unlock_$1.ko
