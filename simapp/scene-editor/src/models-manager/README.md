## 集成方式：

### submodule方式：
1. 将仓库以submodule方式引入项目
2. 配置vue.config.js或者webpack.config.js等的copy插件，将models-manager/assets目录发布到项目的dist产出中。下面以单机版的vue.config.js的chainWebpack为例
    ```
    config.plugin('copy').tap((args) => {
       args[0].push({
           from: 'models-manager/assets',
           to: 'assets',
       });
       return args;
    });
    ```
3. 如果以前给.fbx文件配置了loader，可以选择删除了。
4. 引用的时候填写prefix，需要考虑到config里配置的to目录，如上面例子中ScenarioModelsManager的prefix就传```'/assets/models'```
