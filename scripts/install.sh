#!/bin/sh

optimen_user=optimen
optimen_dir=/var/optimen

echo "check for user '$optimen_user'"
if getent passwd $optimen_user > /dev/null 2>&1; then
	echo "user '$optimen_user' exists"
else
	echo "user '$optimen_user' doesn't exits, create it"
	useradd -UM $optimen_user
fi

echo "check for '$optimen_dir'"
if [ -d $optimen_dir ]; then
	echo "directory '$optimen_dir' exitst'"
else
	mkdir -p $optimen_dir
fi

echo "changing owner of $optimen_dir to $optimen_user"
chown -R $optimen_user:$optimen_user $optimen_dir
