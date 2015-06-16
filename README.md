#MessageWait

### 设计思路 
基于消息的程序架构中，在发送一个request消息后可能会遇到这样的情况
  * 1） 等到response消息，其中response分为肯定回答和否定回答。
  * 2） 在规定时间内没有收到回复。

往往我们的业务中需要等到结果以后再进行其他业务，我提供了两种解决办法：
  * 一种是同步方法，在发送request消息后挂起当前线程，当收到消息或者超时后激活当前线程，根据返回结果，继续处理剩下的业务；
  * 另一种是异步方法，采用回调的方法调用响应函数，维护一个定时器，不停检查是否有超时消息产生，并调用对应回调函数。

详细请查看doc目录中的文档

### 说明 
使用到了poco库和boost库，使用poco库的部分完全可以使用boost代替。

引用的boost和poco库由于比较大，因而没有上传，需要读者自己去下载，工程路径也需要根据实际修改。boost库[下载地址](http://sourceforge.net/projects/boost/files/boost/1.55.0/)，poco库[下载地址](http://pocoproject.org/releases/poco-1.4.7/)
