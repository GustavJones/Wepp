cd $(dirname $0)

WeppPID=0
ApiPID=0
 
trap "echo '' && echo 'Stopping' && kill $ApiPID && kill $WeppPID && exit" SIGINT

./build/src/Application/Wepp &
WeppPID=$!
./build/src/Api/Wepp-Api &
ApiPID=$!
wait
