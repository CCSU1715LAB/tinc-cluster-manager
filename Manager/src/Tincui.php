<?php

namespace addons\tincui;

use app\common\library\Menu;
use think\Addons;

class Tincui extends Addons
{
    //安装插件
    public function install(){
        $menu = include ADDON_PATH . 'tincui' . DS . 'data' . DS . 'menu.php';
        Menu::create($menu);

}
   //卸载插件 
    public function uninstall(){
    Menu::delete('tincui/Singlenwdata');
    Menu::delete('tincui/Nwdata');
    Menu::delete('tincui/netmanagement');
    Menu::delete('tincui/nodemanagement');
    Menu::delete('tincui/events');
}
     //禁用插件
    public function disable(){
        Menu::disable('tincui/Singlenwdata');
        Menu::disable('tincui/Nwdata');
        Menu::disable('tincui/netmanagement');
        Menu::disable('tincui/nodemanagement');
        Menu::disable('tincui/events');
}
    //启动插件
    public function enable(){
        Menu::enable('tincui/Singlenwdata');
        Menu::enable('tincui/Nwdata');
        Menu::enable('tincui/netmanagement');
        Menu::enable('tincui/nodemanagement');
        Menu::enable('tincui/events');
}



}