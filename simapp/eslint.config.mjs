import antfu from '@antfu/eslint-config'

export default antfu(
  {
    typescript: true,
    javascript: true,
    vue: true,
    ignores: [
      '**/node_modules/**/*',
      '**/*/helvetiker_regular.typeface.json',
      '**/*.cpp.js',
    ],
  },
  {
    rules: {
      'curly': [2, 'multi-line'], // 允许单行脚本，多行需大括号
      'no-console': 0, // 允许 console
      'no-empty': 0, // 允许空代码块
      'eqeqeq': 0, // 允许使用 ==
      'no-alert': 0, // 允许使用 alert
      'accessor-pairs': 0, // 允许 class 中 set get 不同时出现
      'max-len': [2, {
        code: 120,
        ignoreStrings: true,
        ignoreUrls: true,
        ignoreRegExpLiterals: true,
        ignoreTemplateLiterals: true,
      }], // 限制每行最大字符数
      'id-match': [2, '^[a-zA-Z0-9_$]{1,35}$'],
      'antfu/if-newline': 0, // 允许单行的 if
      'unused-imports/no-unused-vars': 0, // 未使用的 import 或变量
      'node/prefer-global/process': 0, // 允许使用 process
      'unicorn/prefer-node-protocol': 0, // 允许不加 node:fs 的 node，后续 electron 升级了可加
      'new-cap': [2, {
        newIsCap: false,
        capIsNew: false,
        properties: false,
      }], // 允许 new a_b() 或 MessageBox() 或 this.Box 的名称问题
      'eslint-comments/no-unlimited-disable': 0, // 允许禁用 eslint 规则
      'style/eol-last': [2, 'always'], // 文件结尾必须有空行
      'style/brace-style': [2, '1tbs'], // else 不换行
      'style/space-before-function-paren': [2, 'always'], // 函数名称必加空格
      'style/operator-linebreak': [2, 'after'], // ?: 或 &&|| 等换行时，位于结尾
      // 'style/comma-dangle': [2, 'never'], // 对象、对象、函数等结尾不加逗号
      'ts/no-this-alias': 0, // 允许 this 赋值给变量
      'ts/no-loss-of-precision': 0, // 允许任意浮点数作定量
      'ts/no-use-before-define': 0, // 允许 const 写在调用之后
      'ts/no-namespace': 0, // 使用 namespace 定义
      'ts/ban-ts-comment': 0, // 允许使用 @ts-expect-error
      'vue/no-unused-refs': 0, // 未使用的 template ref
      'vue/block-order': [2, { order: [['script', 'template'], 'style'] }], // 不约束 script 和 template 顺序，style 放在最后
      'vue/custom-event-name-casing': [2, 'kebab-case', { ignores: ['/^[a-z]+\/[a-z]+$/'] }], // vue emit 的事件名称需为中划线
      'vue/component-name-in-template-casing': [2, 'PascalCase', {
        registeredComponentsOnly: true,
      }], // template 中业务组件名称需驼峰，全局组件为中划线
      'vue/require-explicit-emits': 0, // 允许 emit 不注册
      'vue/no-restricted-v-bind': 0, // 允许 v-loading 等语法
      // 'vue/html-self-closing': [1, { html: { void: 'always' } }],
      'jsdoc/check-param-names': 0, // 允许任意 @param 注释
      'jsdoc/require-returns-check': 0, // 允许 @returns 没类型
      'jsdoc/require-returns-description': 0, // 允许 @returns 没描述
      'regexp/no-unused-capturing-group': 0, // 正则中允许无作用的 () 捕获组
    },
  },
)
