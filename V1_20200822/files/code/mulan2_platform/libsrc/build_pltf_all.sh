#! /bin/sh
#
#     foo=/tmp/my.dir/filename.tar.gz 
#
#     We can use these expressions:
#
#     path = ${foo%/*}
#         To get: /tmp/my.dir (like dirname)
#
#     file = ${foo##*/}
#         To get: filename.tar.gz (like basename)
#
#     base (longest match deleted) = ${foo%%.*}
#         To get: filename 
#
#     base (shortest match deleted) = ${foo%.*}
#         To get: filename 
#
#     ext (longets match deleted) = ${foo##*.}
#         To get: tar.gz 
#
#     ext (shortest match deleted) = ${foo#*.}
#         To get: tar.gz 
#


echo --- Building profiles for all products ...

for product in ../products/*
  do

    product_name=${product##*/}
    # Take only 5-digit projects with possible one or two letters for project revision
    case $product_name in                       \
        [1-9][0-9][0-9][0-9][0-9]               \
      | [1-9][0-9][0-9][0-9][0-9][A-Z]          \
      | [1-9][0-9][0-9][0-9][0-9][A-Z][A-Z]     \
      | [1-9][0-9][0-9][0-9][0-9][_][0-9])
            echo --- Product: $product_name
            for profile in ../products/$product_name/config/*.mk
                do
                    profile_name=${profile##*/}
                    if [ $profile_name != "Extra.mk" ]; then                   # skip Extra.mk
                        echo --- Processing profile ${profile_name%%.*} from file : $profile_name ...

                        gmake -s uninstall PRODUCT=$product_name PROFILE=${profile_name%%.*}
                        gmake -s clean     PRODUCT=$product_name PROFILE=${profile_name%%.*}
                        gmake -s install   PRODUCT=$product_name PROFILE=${profile_name%%.*}
                        echo
                        echo
                    fi
                done;
    esac

  done;

#EOF
