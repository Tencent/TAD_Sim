<template>
  <div class="signal-control-phase-actions" :class="{ disabled }">
    <el-icon class="el-icon-plus" @click="handleCommand('addPhase')">
      <plus />
    </el-icon>
    <el-icon class="el-icon-minus" @click="handleCommand('delPhase')">
      <minus />
    </el-icon>
    <!-- <el-icon class="el-icon-back" @click="handleCommand('move', 'pre')"><back /></el-icon>
    <el-icon class="el-icon-right" @click="handleCommand('move', 'next')"><right /></el-icon> -->
    <div class="add-color red" @click="handleCommand('add', 'red')">
      <el-icon class="el-icon-plus">
        <plus />
      </el-icon>
    </div>
    <div class="add-color green" @click="handleCommand('add', 'green')">
      <el-icon class="el-icon-plus">
        <plus />
      </el-icon>
    </div>
    <div class="add-color yellow" @click="handleCommand('add', 'yellow')">
      <el-icon class="el-icon-plus">
        <plus />
      </el-icon>
    </div>
    <div class="del-color" @click="handleCommand('delColor')">
      <div class="green" />
      <div class="red" />
      <div class="yellow" />
      <el-icon class="el-icon-minus">
        <minus />
      </el-icon>
    </div>
  </div>
</template>

<script>
export default {
  name: 'SignalControlPhaseActions',
  props: {
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  methods: {
    handleCommand (command, args) {
      if (this.disabled) return
      this.$emit('command', command, args)
    },
  },
}
</script>

<style scoped lang="less">
.signal-control-phase-actions {
  display: flex;
  align-items: center;
  justify-content: flex-end;
  padding: 12px 10px;
  > * {
    margin-left: 10px;
    cursor: pointer;
  }

  &.disabled {
    > * {
      opacity: 0.5;
      cursor: not-allowed;
    }
  }

  i {
    color: #fff;
    font-weight: bold;
  }
  .add-color {
    position: relative;
    width: 12px;
    height: 12px;
    border: 2px solid #fff;
    box-sizing: border-box;
    &.green {
      background-color: #079d4b50;
      border-color: #079d4b;
    }
    &.yellow {
      background-color: #E3940050;
      border-color: #E39400;
    }
    &.red {
      background-color: #8E001550;
      border-color: #8E0015;
    }
    .el-icon-plus {
      position: absolute;
      right: 0;
      bottom: 0;
      transform: translate(52%, 52%);
    }
  }
  .del-color {
    position: relative;
    width: 12px;
    height: 12px;
    display: flex;
    .green {
      width: 3px;
      height: 100%;
      background-color: #079d4b;
    }
    .yellow {
      width: 3px;
      height: 100%;
      background-color: #E39400;
    }
    .red {
      width: 3px;
      height: 100%;
      background-color: #8E0015;
    }
    .el-icon-minus {
      position: absolute;
      right: 0;
      bottom: 0;
      transform: translate(52%, 52%);
    }
  }
}
</style>
