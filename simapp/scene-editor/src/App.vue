<template>
  <div id="app">
    <router-view />
  </div>
</template>

<script>
export default {
  name: 'App',
}
</script>

<style lang="less">
  @import "@/assets/less/mixins";

  .custom-font-class{
    font-family: Custom Font, sans-serif !important;
    input, button, input::-webkit-input-placeholder {
      font-family: Custom Font, sans-serif !important;
    }
  }

  @font-face {
    font-family: "Custom Font";
    src: url("./assets/font/NotoSansSC-Medium.otf");
  }

  html, body, div, ul, ol, li, p,
  h1, h2, h3, h4, h5, h6 {
    margin: 0;
    padding: 0;
  }

  html, body {
    height: 100%;
    font-family: -apple-system, "PingFang SC", "Microsoft YaHei", "Custom Font", system-ui, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
  }

  body {
    background-color: #020202;
  }

  li {
    list-style: none;
  }

  ::-webkit-scrollbar {
    width: 6px;
    height: 6px;
  }

  ::-webkit-scrollbar-thumb {
    background-color: #484848;
    border-radius: 4px;
  }

  ::-webkit-scrollbar-corner,
  ::-webkit-resizer {
    background-color: transparent;
  }

  .flex-space-between {
    display: flex;
    justify-content: space-between;
  }

  .icon {
    outline: none;
  }

  .el-select-dropdown{
    .el-select-dropdown__item{
      .text-overflow();
      color: @title-font-color;
      &:hover{
        background-color: #242424;
        color: #16d1f3;
      }
    }
  }

  input,
  textarea {
    font-family: -apple-system, "PingFang SC", "Microsoft YaHei", "Custom Font", system-ui, sans-serif !important;
    color: @title-font-color !important;
  }

  input::-webkit-input-placeholder,
  textarea::-webkit-input-placeholder {
    font-family: -apple-system, "PingFang SC", "Microsoft YaHei", "Custom Font", system-ui, sans-serif !important;
    color: @disabled-color !important;
  }

  textarea:disabled, input:disabled{
    // background-color: @hover-bg !important;
    border-color: @title-bd !important;
    color: @title-font-color !important;
    cursor: not-allowed !important;
  }

  button{
    font-family: -apple-system, "PingFang SC", "Microsoft YaHei", "Custom Font", system-ui, sans-serif !important;
  }

  .el-tabs{
    .el-tabs__item{
      color: @title-font-color;
      &:hover{
        color: @active-font-color;
      }
    }
    .is-active{
      color: @active-font-color;
    }
  }

  .el-dialog__wrapper {
    overflow: hidden;
  }

  #app {
    height: 100%;
    font-size: 14px;
  }
</style>
