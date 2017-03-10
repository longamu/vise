if [ $# -ne 3 ]; then
  echo "Usage: `basename $0` webport demoname apiport"
  exit $E_BADARGS
fi

webport=$1
demoname=$2
apiport=$3

# This script is here to be run as screen ./scriptname
# Do not run screen inside of this - problems with loading .bash_profile (setting LD_LIBRARY_PATH)

source ~/.bash_profile

while true; do
    echo -n "starting webserver: "
    date
    echo -n "starting webserver: " >> start.log
    date >> start.log
    python src/ui/web/webserver.py $webport $demoname $apiport u
    sleep 1
done
