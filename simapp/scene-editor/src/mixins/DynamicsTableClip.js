import { cloneDeep, debounce, throttle } from 'lodash-es'

/**
 * 获取不同操作系统的结束符
 * @return {string}
 */
function getEOL () {
  const { platform } = navigator

  if (platform.startsWith('Win')) {
    return '\r\n'
  } else if (platform.startsWith('Mac')) {
    return '\n'
  } else if (platform.startsWith('Linux')) {
    return '\n'
  } else {
    return '\n'
  }
}
const EOL = getEOL()
const SEP = '\t'

let table = null
const selection = null
export default {
  data: () => ({
    cacheObj: {},
    cachTableData: [],
    startCell: null, // 框选左上角单元格或者粘贴时候的坐上角
    endCell: null,
    globalLeftTop: null,
    selectionList: [],
    isEdit: false,
    lineNum: 2,
    rowNum: 2,
    selectionIndex: {
      startRow: -1,
      startCellIndex: -1,
      endRow: -1,
      endCellIndex: -1,
    },
  }),
  computed: {
    isThreeAxis () {
      return !!this.modelValue.u1Axis
    },
    tableList () {
      return this.isEdit ? this.cachTableData : this.tableData
    },
    lineTotable () {
      return this.isEdit ? this.cacheObj.u1Axis.data.length : this.modelValue.u1Axis.data.length
    },
  },
  created () {
    if (this.modelValue.u1Axis) {
      // 三维(1, 1)位置数据填充
      this.rowNum = this.modelValue.u0Axis.data.length + 1
      this.lineNum = this.modelValue.u1Axis.data.length + 2
    } else {
      this.lineNum = 2
      this.rowNum = this.modelValue.u0Axis.data.length
    }
    // console.log('rownum', this.rowNum)
  },
  mounted () {
    this.$nextTick(() => {
      document.addEventListener('paste', this.handlePaste)

      table = this.$refs.tableRef.$el.getElementsByTagName('table')[1]

      table.addEventListener('mousedown', this.tableMouseDown)
      table.addEventListener('mousemove', this.throttleMove)
      document.addEventListener('mouseup', this.tableMouseUp)

      document.addEventListener('copy', this.handleCopy)
      document.addEventListener('click', this.handleClickOutside)
    })
  },
  beforeUnmount () {
    this.isEdit = false
    this.clearSelection()
    table.removeEventListener('mousedown', this.tableMouseDown)
    table.removeEventListener('mousemove', this.throttleMove)
    document.removeEventListener('mouseup', this.tableMouseUp)
    document.removeEventListener('paste', this.handlePaste)
    document.removeEventListener('copy', this.handleCopy)
  },
  methods: {
    // 开始编辑
    toggleEdit (flag) {
      if (flag) {
        this.cachTableData = cloneDeep(this.tableData)
        this.cacheObj = cloneDeep(this.paramsObj)
      }
      // 需要同步列数， 防止修改列数等取消后没更新
      if (!flag) {
        if (this.modelValue.u1Axis) {
          // 三维(1, 1)位置数据填充
          this.rowNum = this.modelValue.u0Axis.data.length + 1
          this.lineNum = this.modelValue.u1Axis.data.length + 2
        } else {
          this.lineNum = 2
          this.rowNum = this.modelValue.u0Axis.data.length
        }
      }

      this.isEdit = flag
    },
    handleClickOutside (event) {
      const isSelf = this.$refs?.tableRef?.$el.contains(event.target)

      if (!isSelf) {
        this.clearSelection()
      }
    },
    // 行数改变
    handleRowChange (value) {
      if (value < 1 || value === '') {
        this.rowNum = 1
      } else if (value > 100) {
        this.rowNum = 100
      }
      const rowNum = this.rowNum
      // 只修改tableData, 相等不修改
      if (rowNum > this.cachTableData.length) {
        if (this.isThreeAxis) {
          // 三维数据,
          for (let i = this.cachTableData.length; i < rowNum; i++) {
            const obj = {
              index: i,
              x: 0,
            }
            for (let j = 0; j < rowNum; j++) {
              obj[`y${j}`] = 0
            }
            this.cachTableData.push(obj)
          }
        } else {
          // 二维数据
          for (let i = this.cachTableData.length; i < rowNum; i++) {
            const obj = {
              x: 0,
              y: 0,
            }
            this.cachTableData.push(obj)
          }
        }
      } else {
        // 二维数据第一列是table的index
        this.cachTableData.splice(rowNum)
      }
      // console.log(this.cachTableData);
      this.$nextTick(() => {
        table = this.$refs.tableRef.$el.getElementsByTagName('table')[1]
        this.tableListToObj()
      })
    },
    debounceRow: debounce(function (value) {
      this.handleRowChange(value)
    }, 800),
    debounceLine: debounce(function (value) {
      this.handleLineChange(value)
    }, 800),
    // 列数改变, 只针对3维数据
    handleLineChange (value) {
      if (value < 2 || value === '') {
        this.lineNum = 2
      } else if (value > 100) {
        this.lineNum = 100
      }
      const lineNum = this.lineNum
      // 三维数据第一列是生成数据, index和x是必须保留的， lineNum不能小于2
      const currentLineNum = Object.keys(this.cachTableData?.[0] || {}).length

      const restLineNum = lineNum - 2
      if (currentLineNum > lineNum) {
        // 截取列数据
        this.cachTableData.forEach((item, index) => {
          for (let i = restLineNum; i < (currentLineNum - 2); i++) {
            delete item[`y${i}`]
          }
        })
      } else {
        // 添加列数据 yx： 0
        this.cachTableData.forEach((item, index) => {
          const obj = { ...item }
          for (let i = (currentLineNum - 2); i < restLineNum; i++) {
            obj[`y${i}`] = 0
          }
          this.cachTableData[index] = obj
          // this.$set(this.cachTableData, index, obj)
        })
      }
      // console.log(this.cachTableData);
      this.$nextTick(() => {
        table = this.$refs.tableRef.$el.getElementsByTagName('table')[1]
        this.tableListToObj()
      })
    },
    clearSelection () {
      this.selectionIndex.startRow = -1
      this.selectionIndex.endRow = -1
      this.selectionIndex.startCellIndex = -1
      this.selectionIndex.endCellIndex = -1
    },
    isBeloneToTd (target) {
      if (target.className === 'value' || target.className.includes('el-tooltip')) {
        return target.offsetParent.className.includes('el-table__cell')
      }
      if (target.tagName === 'TD') {
        return true
      }
    },
    getElTd (target) {
      if (target.tagName === 'TD') {
        return target
      }
      if (target.className === 'value' || target.className.includes('el-tooltip')) {
        return target.closest('td')
      }
    },
    handleCopy (e) {
      if (this.selectionList.length > 0) {
        const text = this.parseToExcel(this.selectionList)
        this.writeToClipboardText(text)
      }
    },
    tableMouseDown (event) {
      if (this.isBeloneToTd(event.target) && this.isEdit) {
        console.log('mousedown')
        this.startCell = this.getElTd(event.target)
        this.endCell = this.getElTd(event.target)
        this.updateSelection(this.startCell, this.endCell)
      }
    },
    throttleMove: throttle(function (event) {
      this.tableMouseMove(event)
    }, 500),
    tableMouseMove (event) {
      if (this.isBeloneToTd(event.target) && this.startCell && this.isEdit) {
        console.log('mousemove')
        this.endCell = this.getElTd(event.target)
        this.updateSelection(this.startCell, this.endCell) // todo 节流
      }
    },
    tableMouseUp (event) {
      if (this.isEdit) {
        console.log('mouseup')
        this.startCell = null
        this.endCell = null
      }
    },
    setCellClass ({ row, column, rowIndex, columnIndex }) {
      if (rowIndex >= this.selectionIndex.startRow &&
        rowIndex <= this.selectionIndex.endRow &&
        columnIndex >= this.selectionIndex.startCellIndex &&
        columnIndex <= this.selectionIndex.endCellIndex) {
        return 'high-light-cell'
      } else {
        return ''
      }
    },
    tableListToObj () {
      const rowData = [...this.cachTableData]
      // 对paramsObj进行深拷贝， 再基于拷贝数据去组装
      const { u0Axis, u1Axis, y0Axis } = this.cacheObj
      let u0AxisData = []
      let u1AxisData = []
      let y0AxissData = []
      // 通过u1Axis判断是否三维数据
      if (u1Axis) {
        u0AxisData = [...rowData.map(v => v.x)]
        u1AxisData = [...Object.values(rowData[0])]
        if (true) {
          u0AxisData.splice(0, 1)
          u1AxisData.splice(0, 2)
        }

        y0AxissData = []
        let index = 1
        while (index <= u1AxisData.length) {
          rowData.forEach((value, idx) => {
            if (idx > 0) {
              y0AxissData.push(value[`y${index - 1}`])
            }
          })
          index++
        }
        u1Axis.data = u1AxisData
      } else {
        u1AxisData = null
        u0AxisData = rowData.map(v => v.x)
        y0AxissData = rowData.map(v => v.y)
      }
      u0Axis.data = u0AxisData
      y0Axis.data = y0AxissData
    },
    updateSelection (start, end) {
      if (!start || !end) {
        console.error(`start ${start}, end ${end}`)
      }
      let startRow = start.closest('tr').rowIndex
      let startCellIndex = start.cellIndex
      let endRow = end.closest('tr').rowIndex
      let endCellIndex = end.cellIndex

      if (startRow > endRow) {
        [startRow, endRow] = [endRow, startRow]
      }
      if (startCellIndex > endCellIndex) {
        [startCellIndex, endCellIndex] = [endCellIndex, startCellIndex]
      }

      const leftTop = table.rows[startRow].cells[startCellIndex]
      const rightBottom = table.rows[endRow].cells[endCellIndex]

      this.globalLeftTop = leftTop
      // this.updateSelectionStyle(leftTop, rightBottom);

      this.selectionList = []
      this.selectionIndex.startRow = startRow
      this.selectionIndex.endRow = endRow
      this.selectionIndex.startCellIndex = startCellIndex
      this.selectionIndex.endCellIndex = endCellIndex

      for (let rowIndex = startRow; rowIndex <= endRow; rowIndex++) {
        const row = table.rows[rowIndex]
        const selectionItem = []
        for (let cellIndex = startCellIndex; cellIndex <= endCellIndex; cellIndex++) {
          selectionItem.push(row.cells[cellIndex].textContent)
        }
        this.selectionList.push(selectionItem)
      }
      // console.table(this.selectionList);
    },
    // 设置框选中部分样式
    updateSelectionStyle (leftTop, rightBottom) {
      const rectLeftTop = leftTop.getBoundingClientRect()
      const rectRightBottom = rightBottom.getBoundingClientRect()
      selection.style.display = 'block'
      selection.style.left = `${rectLeftTop.left}px`
      selection.style.top = `${rectLeftTop.top}px`
      selection.style.width = `${rectRightBottom.right - rectLeftTop.left - 2}px`
      selection.style.height = `${rectRightBottom.bottom - rectLeftTop.top - 2}px`
    },
    // 粘贴时候更新table
    updateTable (list) {
      const startRow = this.globalLeftTop.closest('tr').rowIndex
      const startCellIndex = this.globalLeftTop.cellIndex

      // console.log(startRow, startCellIndex, table.rows.length, table.rows[startRow].cells.length);

      for (let rowIndex = startRow; rowIndex < Math.min(table.rows.length, startRow + list.length); rowIndex++) {
        const row = table.rows[rowIndex]
        const item = list[rowIndex - startRow]
        const length = Math.min(row.cells.length, startCellIndex + item.length)
        for (let cellIndex = startCellIndex; cellIndex < length; cellIndex++) {
          // if (this.isThreeAxis && rowIndex === 0 && cellIndex === 1) {
          //   // 三维数据第一行第2列是-， 需要替换
          //   row.cells[cellIndex].getElementsByClassName('value')[0].innerText = '-'
          // } else {
          //   row.cells[cellIndex].getElementsByClassName('value')[0].innerText = item[cellIndex - startCellIndex];
          // }
          if (this.isThreeAxis) {
            if (cellIndex === 1) {
              if (rowIndex === 0) {
                this.cachTableData[rowIndex].x = '-'
              } else {
                this.cachTableData[rowIndex].x = item[cellIndex - startCellIndex]
              }
            } else {
              this.cachTableData[rowIndex][`y${cellIndex - 2}`] = item[cellIndex - startCellIndex]
            }
          } else {
            if (cellIndex === 1) {
              this.cachTableData[rowIndex].x = item[cellIndex - startCellIndex]
            } else {
              this.cachTableData[rowIndex].y = item[cellIndex - startCellIndex]
            }
          }
        }
      }
      this.$nextTick(() => {
        this.tableListToObj()
      })
    },
    insertYRowEdit (index, pos) {
      const { u0Axis: { data: u0Data }, u1Axis: { data: u1Data }, y0Axis: { data: y0Data } } = this.cacheObj
      if (u1Data.length >= 1000) {
        this.$message({
          showClose: true,
          message: '列表项不能多于1000项',
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
      // console.log(this.cacheObj)
      this.generateTableData(this.cacheObj)
      this.$nextTick(() => {
        table = this.$refs.tableRef.$el.getElementsByTagName('table')[1]
      })
    },
    deleteYRowEdit (index) {
      const xLen = this.cacheObj.u0Axis.data.length
      if (this.cacheObj.u1Axis.data.length <= 2) {
        this.$message({
          showClose: true,
          message: '列表项不能少于2项',
          type: 'warning',
        })
        return
      }
      for (let i = xLen - 1; i >= 0; i--) {
        this.cacheObj.y0Axis.data.splice((index - 1) * xLen + i, 1)
      }
      this.cacheObj.u1Axis.data.splice(index - 1, 1)
      this.rowNum = this.cacheObj.u0Axis.data.length + 1
      this.lineNum = this.cacheObj.u1Axis.data.length + 2
      // console.log(this.cacheObj)
      // 修改了对象再重新生成table
      this.generateTableData(this.cacheObj)
      this.$nextTick(() => {
        table = this.$refs.tableRef.$el.getElementsByTagName('table')[1]
      })
    },
    saveEdit () {
      this.tableListToObj()
      this.$emit('input', this.cacheObj)
      this.$emit('update:modelValue', this.cacheObj)
      this.isEdit = false
    },
    async handlePaste (event) {
      if (this.isEdit) {
        const text = await this.readFromClipboardText()
        if (text) {
          console.log('粘贴文本', JSON.stringify(text))
          const regExp = new RegExp(`${EOL}+$`, 'g')
          const list = this.parseFromExcel(text.replace(regExp, '')) // 去掉多余换行
          const valid = this.checkList(list)

          console.log(list, valid)

          if (!valid) {
            // alert('粘贴内容包含非数字类型内容，请检查后重试');
            this.$message({
              message: '粘贴内容包含非数字类型内容，请检查后重试',
              type: 'error',
              customClass: 'custom-message',
            })
            return
          }

          if (!this.globalLeftTop) {
            // alert('请先选中单元格再进行粘贴');
            this.$message({
              message: '请先选中单元格再进行粘贴',
              type: 'error',
              customClass: 'custom-message',
            })
            return
          }
          this.updateTable(list)
        }
      }
      // 编辑模式下直接将table数据放入， save的时候再emit input去修改paramsObj
    },
    transformArrToObj (rowData, flag = true) {
      // 对paramsObj进行深拷贝， 再基于拷贝数据去组装
      const { u0Axis, u1Axis, y0Axis } = this.cacheObj
      let u0AxisData = []
      let u1AxisData = []
      let y0AxissData = []
      // 通过u1Axis判断是否三维数据
      if (u1Axis) {
        u0AxisData = [...rowData.map(v => v[0])]
        u1AxisData = [...rowData[0]]
        if (true) {
          u0AxisData.splice(0, 1)
          u1AxisData.splice(0, 1)
        }

        y0AxissData = []
        let index = 1
        while (index < rowData.length) {
          rowData.forEach((value, idx) => {
            if (idx > 0) {
              y0AxissData.push(value[index])
            }
          })
          index++
        }
        u1Axis.data = u1AxisData
      } else {
        u1AxisData = null
        u0AxisData = rowData.map(v => v[0])
        y0AxissData = rowData.map(v => v[1])
      }
      u0Axis.data = u0AxisData
      y0Axis.data = y0AxissData
      // console.log(this.cacheObj)
      if (flag) {
        return this.cacheObj
      } else {
        return {
          u0AxisData,
          y0AxissData,
          u1AxisData,
        }
      }
    },
    // 解析粘贴板数据成数组
    parseFromExcel (text) {
      return text.split(EOL).map((item) => {
        return item.split(SEP)
      })
    },
    // 复制表格数据， 处理后放到剪切板上
    parseToExcel (list) {
      let text = ''

      list.forEach((item, index) => {
        if (index !== 0) {
          text += EOL
        }
        text += item.join(SEP)
      })

      return text
    },
    // 数据校验
    checkList (list) {
      let valid = true

      for (const item of list) {
        valid = item.every(val => /^-?\d+(\.\d+)?$/.test(val))
        if (!valid) {
          break
        } else {
          continue
        }
      }
      return valid
    },
    // 读取剪切板数据
    async readFromClipboardText () {
      try {
        const text = await navigator.clipboard.readText()

        return text
      } catch (error) {
        this.$message({
          message: `Error writing clipboard:", ${error}`,
          type: 'error',
          customClass: 'custom-message',
        })
        // console.error("Error reading clipboard:", error);
      }
    },
    // 放置数据到剪切板
    async writeToClipboardText (text) {
      try {
        await navigator.clipboard.writeText(text)
        console.log(text)
      } catch (error) {
        this.$message({
          message: `Error writing clipboard:", ${error}`,
          type: 'error',
          customClass: 'custom-message',
        })
        // console.error("Error writing clipboard:", error);
      }
    },
  },
}
