##线程管理器
from Thread import Thread
class ThreadManager:
    thread_list=[]#线程列表
    XML_FILE_PATH='./xml/tincs.xml'##配置文件路径
    
    ##加载配置文件
    def load_info(self):
        print("---加载线程配置文件---")
    
    ##保存配置文件
    def save_info(self):
        print("---保存线程配置文件---")
    
    ##生成线程
    def assign_thread(self,network_name):
        print(f"正在为{network_name}分配线程")
        ##获取栈顶线程
        top_thread=self.thread_list[-1]
        if top_thread.get_tincs_cnt()<self.Max_tincs_cnt:
            top_thread.add_tincs(network_name)
        else:
            new_thread=Thread(network_name)
            self.thread_list.append(new_thread)
    ##删除线程
    def delete_thread(self,thread_id):
        print("---删除线程---")
        print(f"正在删除线程:{thread_id}")
    ##展示所有线程
    def show_all_threads(self):
        print("---展示所有线程---")    
    
    ##展示单个线程
    def show_single_thread(self,thread_id):
        print("---展示单个线程---")
        print(f"正在展示线程:{thread_id}")
    
    
    
