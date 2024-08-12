<template>
  <div ref="wraper" class="dynamics-table-chart" :class="{ disabled }">
    <template v-if="u1Axis">
      <div class="edit-wrap ">
        <div class="edit-contnet-left edit-wrap-column">
          <div class="edit-item">
            <span>行数</span>
            <el-input
              v-model="rowNum"
              :disabled="!isEdit"
              type="number"
              :min="1"
              :max="100"
              onkeypress="return (/[\d]/.test(String.fromCharCode(event.keyCode)))"
              @input="debounceRow"
            />
          </div>
          <div class="edit-item">
            <span>列数</span>
            <el-input
              v-model="lineNum"
              :disabled="!isEdit"
              type="number"
              :min="1"
              :max="100"
              onkeypress="return (/[\d]/.test(String.fromCharCode(event.keyCode)))"
              @input="debounceLine"
            />
          </div>
        </div>
        <el-icon
          v-if="!isEdit"
          class="el-icon-edit edit-contnet-left"
          @click="toggleEdit(true)"
        >
          <EditPen />
        </el-icon>
      </div>
      <!-- 三维图表 -->
      <el-table
        ref="tableRef"
        :max-height="200"
        :highlight-current-row="false"
        :data="tableList"
        :cell-class-name="setCellClass"
      >
        <el-table-column prop="index" />
        <el-table-column label="XAxis" show-overflow-tooltip>
          <template #default="{ row }">
            <div class="value">
              {{ row.x }}
            </div>
            <!-- <el-input
              v-if="$index > 0"
              v-model.number="inputValue"
              type="number"
              @mousedown.native="inputClick(row.x)"
              @blur="inputBlur('u0Axis', $index - 1)"
            /> -->
          </template>
        </el-table-column>

        <!-- u1Axis.data.length -->
        <template v-for="index in lineTotable" :key="index">
          <el-table-column :label="index.toString()" show-overflow-tooltip>
            <template #header>
              {{ index }}
              <el-dropdown v-if="isEdit" trigger="hover" size="mini">
                <el-icon><Setting /></el-icon>
                <template #dropdown>
                  <el-dropdown-menu placement="bottom">
                    <el-dropdown-item @click="insertYRowEdit(index, 'before')">
                      向前插入一列
                    </el-dropdown-item>
                    <el-dropdown-item @click="insertYRowEdit(index, 'after')">
                      向后插入一列
                    </el-dropdown-item>
                    <el-dropdown-item @click="deleteYRowEdit(index)">
                      删除
                    </el-dropdown-item>
                  </el-dropdown-menu>
                </template>
              </el-dropdown>
            </template>
            <template #default="{ row }">
              <div class="value">
                {{ row[`y${index - 1}`] }}
              </div>
              <!-- <el-input
                v-model.number="inputValue"
                type="number"
                @mousedown.native="inputClick(row[`y${index-1}`])"
                @blur="inputBlur(
                  $index === 0 ? 'u1Axis' : 'y0Axis',
                  $index === 0 ? index - 1 : u0Axis.data.length * (index - 1) + $index - 1,
                )"
              /> -->
            </template>
          </el-table-column>
        </template>
        <!-- <el-table-column width="56px" label="操作">
          <template slot-scope="{ $index }">
            <el-dropdown v-if="$index > 0" size="mini">
              <i class="el-icon el-icon-setting"></i>
              <el-dropdown-menu slot="dropdown" placement="bottom">
                <el-dropdown-item @click.native="insertXRow($index, 'before')">向上插入一行</el-dropdown-item>
                <el-dropdown-item @click.native="insertXRow($index, 'after')">向下插入一行</el-dropdown-item>
                <el-dropdown-item @click.native="deleteXRow($index)">删除</el-dropdown-item>
              </el-dropdown-menu>
            </el-dropdown>
          </template>
        </el-table-column> -->
      </el-table>
      <div v-if="isEdit" class="custom-button form-section-foot">
        <el-button size="small" plain @click="toggleEdit(false)">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button size="small" @click="saveEdit">
          {{ $t('operation.ok') }}
        </el-button>
      </div>
      <div class="chart-axis-label">
        <div class="x-axis">
          X:{{ u0Axis.dispName }}{{ getUnit(u0Axis) }}
        </div>
        <div class="y-axis">
          Y:{{ u1Axis.dispName }}{{ getUnit(u1Axis) }}
        </div>
        <div class="z-axis">
          Z:{{ y0Axis.dispName }}{{ getUnit(y0Axis) }}
        </div>
      </div>
      <DynamicsThreeDLineChart
        :x-axis="u0Axis"
        :y-axis="u1Axis"
        :z-axis="y0Axis"
        :width="width"
      />
    </template>
    <template v-else-if="u0Axis">
      <div class="edit-wrap">
        <div class="edit-contnet-left">
          <span>行数</span>
          <el-input
            v-model="rowNum"
            :disabled="!isEdit"
            type="number"
            :min="1"
            :max="100"
            onkeypress="return (/[\d]/.test(String.fromCharCode(event.keyCode)))"
            @input="debounceRow"
          />
        </div>
        <el-icon
          v-if="!isEdit"
          class="el-icon-edit edit-contnet-left"
          @click="toggleEdit(true)"
        >
          <EditPen />
        </el-icon>
      </div>
      <!-- 二维图表 -->
      <el-table
        ref="tableRef"
        :max-height="200"
        :highlight-current-row="false"
        :data="tableList"
        :cell-class-name="setCellClass"
      >
        <el-table-column type="index" />
        <el-table-column label="X Axis" show-overflow-tooltip>
          <template #default="{ row }">
            <div class="value">
              {{ row.x }}
            </div>
          </template>
        </el-table-column>
        <el-table-column label="Y Axis" show-overflow-tooltip>
          <template #default="{ row }">
            <div class="value">
              {{ row.y }}
            </div>
            <!-- <el-input
              v-model="inputValue"
              type="number"
              @mousedown.native="inputClick(row.y)"
              @blur="inputBlur('y0Axis', $index)"
            /> -->
          </template>
        </el-table-column>
        <!-- <el-table-column width="56px" label="操作">
          <template slot-scope="{ $index }">
            <el-dropdown size="mini" placement="bottom">
              <i class="el-icon el-icon-setting"></i>
              <el-dropdown-menu slot="dropdown" placement="bottom">
                <el-dropdown-item @click.native="insertRow($index, 'before')">向上插入一行</el-dropdown-item>
                <el-dropdown-item @click.native="insertRow($index, 'after')">向下插入一行</el-dropdown-item>
                <el-dropdown-item @click.native="deleteRow($index)">删除</el-dropdown-item>
              </el-dropdown-menu>
            </el-dropdown>
          </template>
        </el-table-column> -->
      </el-table>
      <div v-if="isEdit" class="custom-button form-section-foot">
        <el-button size="small" plain @click="toggleEdit(false)">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button size="small" @click="saveEdit">
          {{ $t('operation.ok') }}
        </el-button>
      </div>
      <div class="chart-axis-label">
        <div class="x-axis">
          X:{{ u0Axis.dispName }}{{ getUnit(u0Axis) }}
        </div>
        <div class="y-axis">
          Y:{{ y0Axis.dispName }}{{ getUnit(y0Axis) }}
        </div>
      </div>
      <DynamicsLineChart
        :x-axis="u0Axis"
        :y-axis="y0Axis"
        :width="width"
      />
    </template>
  </div>
</template>

<script>
import { mapMutations } from 'vuex'
import cloneDeep from 'lodash/cloneDeep'
import _ from 'lodash'
import DynamicsLineChart from '@/components/charts/dynamics-line-chart.vue'
import DynamicsThreeDLineChart from '@/components/charts/dynamics-three-d-line-chart.vue'
import DynamicsTableClip from '@/mixins/DynamicsTableClip'

export default {
  name: 'DynamicsTableChart',
  components: { DynamicsThreeDLineChart, DynamicsLineChart },
  mixins: [DynamicsTableClip],
  props: {
    modelValue: {
      type: Object,
      required: true,
    },
    width: {
      type: Number,
      default: 190,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      preInputValue: 0,
      inputValue: 0,
      paramsObj: {},
      cacheObj: {},
      tableData: [],
      u0Axis: null,
      u1Axis: null,
      y0Axis: null,
      startPosition: {
        row: 1,
        column: 1,
      },
      endPosition: {
        row: 1,
        column: 1,
      },
    }
  },
  watch: {
    modelValue: {
      handler () {
        this.paramsObj = this.modelValue
      },
      deep: true,
    },
    paramsObj: {
      handler () {
        this.generateTableData(this.paramsObj)
        // 需要同步列数， 防止修改列数等取消后没更新
        if (this.paramsObj.u1Axis) {
          // 三维(1, 1)位置数据填充
          this.rowNum = this.paramsObj.u0Axis.data.length + 1
          this.lineNum = this.paramsObj.u1Axis.data.length + 2
        } else {
          this.lineNum = 2
          this.rowNum = this.paramsObj.u0Axis.data.length
        }
      },
      deep: true,
    },
  },
  created () {
    this.paramsObj = this.modelValue
  },
  mounted () {},
  methods: {
    ...mapMutations('planners', ['setIsDirty']),
    editObj (rowData) {
      this.cacheObj = cloneDeep(this.paramsObj)
      const { u0Axis, u1Axis, y0Axis } = this.cacheObj
      const {
        u0AxisData,
        y0AxissData,
        u1AxisData,
      } = this.transformArrToObj(rowData, false)
      console.log(u0AxisData, y0AxissData, u1AxisData)
      if (u1AxisData) {
        if (this.startPosition.row === 0) {
          const sliceEndIndex = _.min([this.lineNum - (this.startPosition.column), u1AxisData.length])
          const restArr = u1AxisData.slice(0, sliceEndIndex)
          u1Axis.data.splice(this.startPosition.column, sliceEndIndex, ...restArr)
        }
        if (this.startPosition.column === 0) {
          const sliceEndIndex = _.min([this.rowNum - (this.startPosition.row), u0AxisData.length])
          const restArr = u0AxisData.slice(0, sliceEndIndex)
          u0Axis.data.splice(this.startPosition.row, sliceEndIndex, ...restArr)
        }
        // 中间的数据需要分别将新旧的一维数据转成表格对应二维数据
        const oldYaxis = this.transArrToArr(y0Axis, u1Axis.data.length, u0Axis.data.length)
        const newYaxis = this.transArrToArr(y0AxissData, u1AxisData.length, u0AxisData.length)
      } else {
        // 二维， 直对u0AxisData，y0AxissData进行修改
        // 根据行数和选择的row， 计算需要替换的数据范围
        // row为splice的起点， 需要计算终点
        const sliceEndIndex = _.min([this.rowNum - (this.startPosition.row), u0AxisData.length])
        // column = 1 决定只修改u0AxisData
        if (this.startPosition.column === 0) {
          const restArr = u0AxisData.slice(0, sliceEndIndex)
          u0Axis.data.splice(this.startPosition.row, sliceEndIndex, ...restArr)
        }
        const restYArr = y0AxissData.slice(0, sliceEndIndex)
        y0Axis.data.splice(this.startPosition.row, sliceEndIndex, ...restYArr)
      }
      console.log(this.cacheObj)
    },
    transArrToArr (rowData, rowWith, columnHeight) {
      const arr = Array.from({ length: columnHeight })
      arr.fill([])
      // const restArr = arr.map((chilArr, index) => {
      //   chilArr.push(rowData[index * columnHeight])
      // })
      let index = 0
      while (index < columnHeight) {
        for (let i = 0; i < rowWith; i++) {
          arr[index].push(rowData[index * columnHeight + i])
        }
        index++
      }
      console.log(arr)
      return arr
    },
    insertYRow (index, pos) {
      const { u0Axis: { data: u0Data }, u1Axis: { data: u1Data }, y0Axis: { data: y0Data } } = this
      if (u1Data.length >= 150) {
        this.$message({
          showClose: true,
          message: '列表项不能多于150项',
          type: 'warning',
        })
        return
      }
      index -= 1
      let newIndex = 0
      if (pos === 'before') {
        if (index !== 0) {
          newIndex = index
        }
      } else if (pos === 'after') {
        newIndex = index + 1
      }
      u1Data.splice(newIndex, 0, this.calcDataValue(u1Data, newIndex))
      const xLen = u0Data.length
      const newY0DataList = []
      for (let i = 0; i < xLen; i++) {
        newY0DataList.push(this.clacInsertYNewVal(y0Data, newIndex, xLen, i))
      }
      for (let i = 0; i < xLen; i++) {
        const y0Index = newIndex * xLen + i
        y0Data.splice(y0Index, 0, newY0DataList[i])
      }
      this.rowNum = u0Data.length + 1
      this.lineNum = u1Data.length + 2
    },
    clacInsertYNewVal (data, newIndex, xLen, i) {
      const len = data.length
      let newData = 0
      if (newIndex === 0) {
        newData = data[i]
      } else if (newIndex === len) {
        newData = data[len - i - 1]
      } else {
        newData = (data[newIndex * xLen + i] + data[(newIndex - 1) * xLen + i]) / 2
      }
      if (Number.isNaN(newData)) {
        newData = 0
      }
      return newData
    },
    deleteYRow (index) {
      const xLen = this.u0Axis.data.length
      if (this.u1Axis.data.length <= 2) {
        this.$message({
          showClose: true,
          message: '列表项不能少于2项',
          type: 'warning',
        })
        return
      }
      for (let i = xLen - 1; i >= 0; i--) {
        this.y0Axis.data.splice((index - 1) * xLen + i, 1)
      }
      this.u1Axis.data.splice(index - 1, 1)
      this.rowNum = this.u0Axis.data.length + 1
      this.lineNum = this.u1Axis.data.length + 2
    },
    insertXRow (index, pos) {
      const { u0Axis: { data: u0Data }, u1Axis: { data: u1Data }, y0Axis: { data: y0Data } } = this
      if (u0Data.length >= 150) {
        this.$message({
          showClose: true,
          message: '列表项不能多于150项',
          type: 'warning',
        })
        return
      }
      index -= 1
      let newIndex = 0
      if (pos === 'before') {
        if (index !== 0) {
          newIndex = index
        }
      } else if (pos === 'after') {
        newIndex = index + 1
      }
      u0Data.splice(newIndex, 0, this.calcDataValue(u0Data, newIndex))
      const zLen = u1Data.length
      const xLen = u0Data.length
      for (let i = 0; i < zLen; i++) {
        const y0Index = i * xLen + newIndex
        y0Data.splice(y0Index, 0, this.calcDataValue(y0Data, y0Index))
      }
    },
    deleteXRow (index) {
      const xLen = this.u0Axis.data.length
      if (xLen <= 2) {
        this.$message({
          showClose: true,
          message: '列表项不能少于2项',
          type: 'warning',
        })
        return
      }
      const yLen = this.u1Axis.data.length
      for (let i = yLen - 1; i >= 0; i--) {
        this.y0Axis.data.splice(i * xLen + index - 1, 1)
      }
      this.u0Axis.data.splice(index - 1, 1)
    },
    insertRow (index, pos) {
      const { u0Axis: { data: u0Data }, y0Axis: { data: y0Data } } = this
      const xLen = u0Data.length
      if (xLen >= 150) {
        this.$message({
          showClose: true,
          message: '列表项不能多于150项',
          type: 'warning',
        })
        return
      }
      let newIndex = 0
      if (pos === 'before') {
        if (index !== 0) {
          newIndex = index
        }
      } else if (pos === 'after') {
        newIndex = index + 1
      }
      u0Data.splice(newIndex, 0, this.calcDataValue(u0Data, newIndex))
      y0Data.splice(newIndex, 0, this.calcDataValue(y0Data, newIndex))
    },
    calcDataValue (data, index) {
      const len = data.length
      let newData = 0
      if (index === 0) {
        newData = data[0]
      } else if (index === len) {
        newData = data[len - 1]
      } else {
        newData = (data[index - 1] + data[index]) / 2
      }
      if (Number.isNaN(newData)) {
        newData = 0
      }
      return newData
    },
    deleteRow (index) {
      if (this.u0Axis.data.length <= 2) {
        this.$message({
          showClose: true,
          message: '列表项不能少于2项',
          type: 'warning',
        })
        return
      }
      this.u0Axis.data.splice(index, 1)
      this.y0Axis.data.splice(index, 1)
    },
    inputClick (value) {
      setTimeout(() => {
        this.preInputValue = value
        this.inputValue = value
      }, 20)
    },
    inputBlur (key, index) {
      const value = this.inputValue * 1
      if (Number.isNaN(value) || value === this.preInputValue) {
        return
      }
      const { data } = this[key]
      if (['u0Axis', 'u1Axis'].includes(key)) {
        if (
          (data[index - 1] !== undefined && value < data[index - 1]) ||
          (data[index + 1] !== undefined && value > data[index + 1])
        ) {
          this.$message({
            showClose: true,
            message: `${key === 'u0Axis' ? 'X axis' : 'Y axis'}轴值要求严格升序`,
            type: 'warning',
          })
          return
        }
      }
      this.$set(this[key].data, index, value)
      this.$emit('input', this.paramsObj)
      this.$emit('update:modelValue', this.paramsObj)
      this.setIsDirty(true)
    },
    generateTableData (objData) {
      // console.log(objData)
      const retData = []
      const { u0Axis, u1Axis, y0Axis } = objData
      this.u0Axis = u0Axis
      this.u1Axis = u1Axis
      this.y0Axis = y0Axis
      if (u1Axis) {
        const row1 = {
          index: 'YAxis',
          x: '-',
        }
        u0Axis.data.forEach((x, xIndex) => {
          const row = {
            index: xIndex + 1,
            x,
          }
          u1Axis.data.forEach((y, yIndex) => {
            if (xIndex === 0) {
              row1[`y${yIndex}`] = y
            }
            row[`y${yIndex}`] = y0Axis.data[yIndex * u0Axis.data.length + xIndex]
          })
          if (xIndex === 0) {
            retData.push(row1)
          }
          retData.push(row)
        })
      } else {
        u0Axis.data.forEach((item, index) => {
          retData.push({
            x: item,
            y: y0Axis.data[index],
          })
        })
      }
      if (this.isEdit) {
        this.cachTableData = retData
      } else {
        this.tableData = retData
      }
    },
    getUnit (axis) {
      const { unit } = axis
      if (unit && unit !== 'null' && unit !== '[]') {
        return `(${unit})`
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";
.dynamics-table-chart {
  // position: relative;
  width: 100%;
  :deep(.high-light-cell) {
    // position: absolute;
    border: 1px solid blue;
    background-color: #4b8cff !important;
    opacity: 0.25;
    // pointer-events: none;
    // user-select:none;
  }
  &.disabled {
    pointer-events: none;

    .dynamics-line-chart {
      pointer-events: auto;
    }
  }
  .edit-wrap {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px;
    .edit-contnet-left {
      display: flex;
      align-items: center;
      span {
        width: 36px;
        flex: 0 0 36px;
      }
      :deep(.el-input) {
        width: 100px;
        margin: 0 10px;
      }
    }
    .el-icon-edit {
      flex: 0 0 1;
    }
  }
  .edit-wrap-column {
    flex-wrap: wrap;
    flex: 1;
    .edit-item {
      display: flex;
      align-items: center;
      margin-bottom: 8px;
    }
  }
  .el-table {
    :deep(td) {
      user-select:none;
    }
    :deep(th) {
      font-weight: 600 !important;
      .cell {
        display: flex;
        align-items: center;
        gap: 3px;
      }
    }
    :deep(.cell) {
      // overflow: hidden;
      // position: relative;
      // height: 24px;
      // line-height: 24px;

      .value {
        width: 100%;
        height: 24px;
        line-height: 24px;
        overflow: hidden;
        white-space: nowrap;
        text-overflow: ellipsis;
        // position: absolute;
        padding: 0 6px;
        color: @global-font-color;
        border-bottom: 1px solid transparent;
        box-sizing: border-box;
      }

      // .el-input {
      //   position: relative;
      //   z-index: 1;

      //   input {
      //     opacity: 0;
      //     border: none;
      //     border-bottom: 1px solid transparent;
      //     background-color: transparent;
      //     height: 22px;
      //     line-height: 22px;
      //     padding: 0 0;
      //     width: 100%;

      //     &:focus {
      //       opacity: 1;
      //       border-bottom: 1px solid #c2c2c2;
      //       background-color: @third-backgroundColor;
      //     }

      //     &::-webkit-inner-spin-button {
      //       display: none;
      //     }
      //   }
      // }

      .el-icon {
        color: @global-font-color;
        display: block;
      }
    }
  }

  .chart-axis-label {
    margin: 10px 0;
    line-height: 1.2;
    font-size: 12px;
    color: @global-font-color;
  }
}
</style>
