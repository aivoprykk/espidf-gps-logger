<template>
  <v-container>
    <v-row align="center" class="list px-3 mx-auto">
    <v-col>
      <v-card class="mx-auto" tile>
        <v-card-title>Logger Configuration</v-card-title>
        <v-data-table
          :headers="headers"
          :items="items"
          item-key="name"
          :items-per-page="itemsPerPage"
          :disable-pagination="true"
          :hide-default-footer="true"
          :hide-default-header="true"
          :loading="loadTable"
          loading-text="Loading data from device ..."
          fixed-header
          height="85vh"
        >
        <template v-slot:[`item.desc`]="{ item }">
          <div v-html="item.desc"></div>
        </template>
        <template v-slot:[`item.value`]="{ item }">
            <span v-if="item && item.toggles">
              <v-btn-toggle
                style="display: table-cell"
                v-model="item.tvalue"
                v-on:change="updateConf(item)"
                multiple
              >
                <v-btn v-for="(t, i) in item.toggles" :key="i" :value="i" v-on:click="updateToggle(item, i)">
                  {{ t.title }}
                </v-btn>
              </v-btn-toggle>
            </span>
            <span v-else-if="item && item.values">

            <v-select
              v-model="item.value"
              label="Select"
              :items="item.values"
              hide-details
              single-line
              :suffix="item.ext"
              variant="outlined"
              @update:modelValue="updateConf(item)"
            ></v-select>

            </span>
            <span v-else>

            <span v-if="item.type != 'bool'">
              <v-text-field
                v-model="item.value"
                label="Edit"
                single-line
                :suffix="item.ext"
                variant="outlined"
                hide-details
                @keydown.enter="updateConf(item)"
              ></v-text-field>
            </span>
            <span v-else>
              <v-switch
                v-model="item.value"
                color="primary"
                hide-details
                v-on:change="updateConf(item)"
              ></v-switch>
            </span>

            </span>
          </template>
        </v-data-table>
      </v-card>
    </v-col>
  </v-row>
  </v-container>
  <v-snackbar
      v-model="snackbar"
      :timeout="snackbarTimeout"
    >
      {{ snackbarText }}

      <template v-slot:actions>
        <v-btn
          color="blue"
          variant="text"
          @click="snackbar = false"
        >
          Close
        </v-btn>
      </template>
    </v-snackbar>
</template>

<script>

import axios from '../service/api-client.js'
export default {
  name: "config",
  data() {
    return {
      selected: [],
      loadTable: true,
      conf: false,
      confColor: '',
      confText: '',
      items: [],
      title: '',
      itemsPerPage: -1,
      headers: [
        { title: 'Name', align: 'start', key: 'name', width: '18%' },
        { title: 'Value', align: 'end', key: 'value', width: '40%', sortable: false },
        { title: 'Description', align: 'end', key: 'desc', width: '40%', sortable: false }
      ],
      snackbar: false,
      snackbarText: '',
      snackbarTimeout: 1000,
    };
  },
  methods: {
    onlynumeric(v) { return (typeof v == 'number' && !isNaN(v)) ? true : 'Only numeric allowed!' },
    save() { return false },
    cancel() { return false },
    open() {
      this.conf = true
      this.confColor = 'info'
      this.confText = 'Dialog opened'
    },
    close() { console.log('Dialog closed') },
    getAll() {
      return axios.get('/config');
    },
    update(id) {
      var val;
      if(id.selected!=null) {
        id.val = id.selected.value;
      }
      var i = {
        name: id.name,
        value: ((id.type == 'bool') ? (id.value ? 1 : 0) : (id.type == 'int'||id.type == 'float') ? (!isNaN(id.value) ? parseFloat(id.value) : 0) : id.value)
      };
      return axios.post(`/config/${i.name}`, i);
    },
    retrieveConfig() {
      this.getAll()
        .then((response) => {
          var d = response.data;
          if (d && d.data && typeof d.data == 'object') {
            d = d.data;
          }
          this.items = d.map(this.getDisplayConf);
          this.loadTable = false;
        })
        .catch((e) => {
          console.log(e);
        });
    },
    refreshList() {
      this.retrieveConfig();
    },
    updateToggle(id) {
      console.log(id);
      if(id.tvalue) {
        var val = 0;
        id.toggles.forEach((v, i) => {
          if(id.tvalue.includes(i)){
            val |=(1<<i);
          }
          else {
            val &=~(1<<i);
          }
        });
        if(id.value != val) {
          // console.log('updateToggle ' + id.name + ': ' + id.value + ' --> ' + val);
          id.value = val;
          this.updateConf(id);
        }
      }
    },
    updateConf(id, val) {
      console.log(id);
      console.log(val);
      this.update(id, val)
        .then(() => {
          this.snackbar = true;
          this.snackbarText = id.name + " updated.";
          this.refreshList();
        })
        .catch((e) => {
          this.refreshList();
          this.snackbar = true;
          this.snackbarText = id.name + " update failed.";
          console.log(e);
        });
    },
    getDisplayConf(d) {
      var toggles = [];
      if(d.toggles) {
        toggles = d.toggles.map((t, i) => {
          if(d.value&t.value) return i;
        });
      }
      return {
        name: d.name,
        value: d.type && d.type == 'bool' ? (d.value == 1 ? true : false) : d.value,
        desc: d.info,
        type: d.type,
        values: d.values ? d.values : null,
        toggles: d.toggles ? d.toggles : null,
        selected: d.values ? {'name':d.name, 'value':d.value, 'type': d.type } : null,
        tvalue: toggles,
      };
    },
  },
  watch: {
    items: {
      handler(val, oldVal) {
        //console.log(oldVal + ' --> ' + val)
        if (!oldVal.length || !val.length) {
          return;
        }
        var a, b;
        for (var i = 0, j = val.length; i < j; ++i) {
          a = val[i];
          b = oldVal[i];
          if (a.name == b.name) {
            if (a.value !== b.value) {
              console.log(a.name + ' --> ' + b.name);
              break;
            }
          }
        }
      },
      deep: true
    }
  },
  mounted() {
    axios.defaults.baseURL = window.location.origin + '/api/v1';
    this.retrieveConfig();
  },
};
</script>
<style>
.v-field__input,
.v-text-field__suffix {
  min-height: 0 !important;
}
.v-data-table-footer {
  display: none !important;
}
</style>
