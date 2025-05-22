cd $(dirname $0)

trap "echo '' && echo 'Stopping' && exit" SIGINT

while [[ true ]]; do
  ./build/src/Application/Wepp
done

