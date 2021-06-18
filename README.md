## A persistent httpserver
We could storage your httpserver informantion.

## BUILD
Befor you build the code ,you need to keep the rocksdb intalled.

### Mac
- `brew install rocksdb`
- `make`
### Linux
- `cd rocksdb;make shared_lib -j; cp librocksdb* /user/lib64`
- `cd PersistentHttpserver; make`

## RUN
- first console: `./httpserver`
- second console: 
  ```shell
  ╰─$ curl -d "value=firstvalue" 127.0.0.1:7999/test/setvalue
  { "result": ok }%                                                                                                                                                            
  # reboot the httpserver, then you could read the data that you set before.
  
  ╭─zhanghuigui ~ 
  ╰─$ curl -d "value=firstvalue" 127.0.0.1:7999/test/getvalue
  { "result": firstvalue }%  
  ```


