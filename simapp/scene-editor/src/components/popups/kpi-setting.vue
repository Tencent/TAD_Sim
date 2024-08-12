<template>
  <div v-loading="loading" class="entity-mgr">
    <section class="entity-mgr-workshop">
      <div class="entity-mgr-workshop-keyword">
        <el-select
          id="kpi-category-select"
          v-model="categoryId"
          class="kpi-search-category"
        >
          <el-option
            v-for="item in specialKpiCategoryOptions"
            :id="`kpi-category-option-${item.name}`"
            :key="item.id"
            :label="$t(item.name)"
            :value="item.id"
          />
        </el-select>
        <el-input
          id="kpi-search-name"
          v-model="keyword"
          class="kpi-search-name"
          prefix-icon="search"
          :placeholder="($t('operation.searchName'))"
        />
      </div>
      <ul id="kpi-list" class="entity-mgr-workshop-list">
        <li
          v-for="(item, index) in list"
          :id="`kpi-node-${index}`"
          :key="item.name"
          class="entity-mgr-workshop-list-item"
          :class="{ 'entity-mgr-workshop-list-item-active': index === currentIndex }"
          @click="handleSelect(index)"
        >
          <div :id="`kpi-node-${getKpiName(item.name)}`">
            <span class="entity-mgr-workshop-list-item-name" :title="$kpiAlias(item)">{{ $kpiAlias(item) }}</span>
            <span class="entity-mgr-workshop-list-item-icon">
              <span class="entity-mgr-workshop-list-item-preset-icon">{{ $t('preset') }}</span>
            </span>
          </div>
        </li>
      </ul>
    </section>
    <section class="entity-mgr-form">
      <KpiFormNew
        v-if="currentFormComponent"
        :value="form"
        :disabled="true"
        :hide-score-setting="true"
        @change="handleKpiDetailChange"
      />
    </section>
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'
import KpiFormNew from './kpi-form-new/index.vue'
import KpiMixin from '@/components/popups/kpi-form-new/mixin'

export default {
  name: 'KpiSetting',
  components: {
    KpiFormNew,
  },
  mixins: [
    KpiMixin,
  ],
  data () {
    return {
      keyword: '',
      categoryId: -1,
      currentIndex: -1,
      form: null,
    }
  },
  computed: {
    ...mapState('kpi', [
      'loading',
      'kpiList',
    ]),
    specialKpiCategoryOptions () {
      return this.kpiCategoryOptions.concat([{ id: '未分类', name: 'kpi.noCategory' }])
    },
    list () {
      return this.kpiList.filter((item) => {
        if (this.categoryId === -1) {
          return true
        } else if (this.categoryId === '未分类') {
          const labels = item.category?.labels || []
          return labels.length === 0
        } else {
          const labels = item.category?.labels || []
          return labels.includes(this.categoryId)
        }
      }).filter((item) => {
        const kpiName = this.$kpiAlias(item)
        if (kpiName) {
          return kpiName.toLowerCase().includes(this.keyword.toLowerCase())
        }
        return false
      })
    },
    currentFormComponent () {
      if (this.form) {
        return this.form
      }
      return null
    },
  },
  watch: {
    list (val) {
      if (val && val.length) {
        this.handleSelect(0)
      } else {
        this.handleSelect(-1)
      }
    },
  },
  async created () {
    await this.getKpiList()
    if (this.list[0]) {
      this.handleSelect(0)
    }
  },
  methods: {
    ...mapActions('kpi', [
      'getKpiList',
    ]),
    getKpiName (name) {
      return name
    },
    async handleSelect (index) {
      this.currentIndex = index
      this.form = this.list[index]
    },
    handleKpiDetailChange (newKpiDetail) {
      this.getKpiList()
      this.form = newKpiDetail
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";
  @import "@/assets/less/entity-mgr";

  .entity-mgr-workshop-keyword {
    display: flex;
    justify-content: space-between;

    .kpi-search-category {
      width: 240px;
    }

    .kpi-search-name {
      margin-left: 3px;
    }
  }

  .entity-mgr-workshop-list-item-preset-icon {
    margin-left: 10px;
    padding: 0 2px;
    font-size: 12px;
    line-height: 12px;
    color: @disabled-color;
    border: 1px solid @disabled-color;
    border-radius: 1px;
  }
</style>
