<template>
  <v-container>
    <v-row align="center" class="list px-3 mx-auto">
    <v-col >
    <v-card class="mx-auto" tile>
    <v-data-table
      :headers="headers"
      :items="items"
      item-value="name"
      :items-per-page="itemsPerPage"
      v-model:sort-by="sortBy"
      class="elevation-1"
      :loading="loadTable"
      loading-text="Loading files from device ..."
      :disable-pagination="true"
      :hide-default-footer="true"
      :hide-default-header="false"
      v-model="selected"
      fixed-header
      height="85vh"
    >
    <template v-slot:top>
      <v-toolbar flat >
        <v-toolbar-title>Files</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-col class="mx-auto" cols="3" sm="3">
        <span v-if="selected.length">
          <v-select
            v-model="bulkselectfields.value"
            @update:modelValue="bulkAction()"
            :items=bulkselectfields.items
            hide-details
            density="compact"
          ></v-select>
        </span>
        </v-col>
        <v-col class="mx-auto" cols="5" sm="5">
          <v-file-input
            id="file"
            v-model="files"
            color="deep-purple-accent-4"
            show-size
            label="Upload file"
            @change="selectFile"
            hide-details
              single-line
          >
          </v-file-input>
        </v-col>
        <span v-if="files.length">
        <v-col class="mx-auto">
          <v-btn small @click="uploadFiles" icon="$upload" >
            <v-icon right>$upload</v-icon>
          </v-btn>
        </v-col>
        </span>
      </v-toolbar>
    </template>
    <template v-slot:[`item.name`]="{ item }">
      <td>
      <span v-if="item.type != 'd' && item.name != 'config.txt'">
      <v-checkbox-btn
        :model-value="isSelected(item)"
        color="primary"
        @update:model-value="toggleSelect(item)"
      ></v-checkbox-btn>
      </span>
      </td>
      <td>
        <span v-if="item">
        <span v-if="item.type != 'd'">
        <a :href="getLink(item.name)">{{ item.name }}</a>
        </span>
        <span v-else>
        <span @click="go(item.name)">{{ item.name }}</span>
        </span>
        </span>
      </td>
    </template>
    <template v-slot:[`item.size`]="{ item }">
      <span v-if="item">
      <span v-if="!item.size && item.type != 'd'">
        -
      </span>
      <span v-if="item.size && item.size > 1024 * 1024">
        {{ (item.size / 1024 / 1024) | 0 }} MB
      </span>
      <span v-else-if="item.size && item.size > 1024">
        {{ (item.size / 1024) | 0 }} KB
      </span>
      <span v-else>
        {{ item.size }}
      </span>
      </span>
    </template>
    <template v-slot:[`item.actions`]="{ item }">
        <span v-if="item">
        <span v-if="item.type != 'd' && item.name != 'config.txt'">
        <v-icon small @click="deleteFile(item.name)" icon="$delete" style="margin-right:0.5em"></v-icon>
        <v-icon small @click="archiveFile(item.name)" icon="$archive"></v-icon>
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
  <v-dialog
      v-model="dialog"
      width="auto"
      ref="dlg"
      v-on:keyup.esc="dialogResult(false)"
      v-on:keyup.enter="dialogResult(true)"
  >
    <v-card>
        <v-card-text>
          <div>
          {{ dialogText }}
          </div>
        </v-card-text>
        <v-card-actions>
          <v-btn color="primary" variant="text" @click="dialogResult(true)">Ok</v-btn>
          <v-btn color="primary" variant="text" @click="dialogResult(false)">Cancel</v-btn>
        </v-card-actions>
    </v-card>
  </v-dialog>
</template>

<script>

import axios from '../service/api-client.js'
export default {
  name: 'files',
  data() {
    return {
      files: [],
      currentFile: undefined,
      progress: 0,
      loadTable: true,
      itemsPerPage: -1,
      selected: [],
      bulkselectfields: {
        value: 0,
        items: [
        { title: 'Action', value: 0, hidden: true },
        { title: 'Delete', value: 1 },
        { title: 'Archive', value: 2 }
      ]},
      items: [],
      title: '',
      headers: [
        { title: 'Name', align: 'start', key: 'name' },
        { title: 'Size', align: 'end', key: 'size' },
        { title: 'Date', align: 'end', key: 'date' },
        { title: 'Type', align: 'end', key: 'type' },
        { title: 'Actions', align: 'end', key: 'actions', sortable: false },
      ],
      sortBy: [{ key: 'date', order: 'desc' }],
      snackbar: false,
      snackbarText: '',
      snackbarTimeout: 1000,
      dialog: false,
      dialogText: '',
      functionResolve: null
    };
  },
  methods: {
    isSelected(item) {
      return this.selected.includes(item.name);
    },
    toggleSelect(item) {
      if (this.selected.includes(item.name)) {
        this.selected = this.selected.filter(i => i !== item.name);
      } else {
        this.selected.push(item.name);
      }
    },
    bulkAction() {
      // console.log(this.bulkselectfields.value);
      if (this.bulkselectfields.value == 1) {
        var n = '';
        this.selected.forEach((item) => {
          for(var i=0; i<this.items.length; i++) {
            if (this.items[i].name == item && this.items[i].type != 'd') {
              if(n.length) n += '|';
              n += item;
              break;
            }
          }
        });
        console.log('delete:' + n);
        if(n.length > 0) this.deleteFile(n);
      } else if (this.bulkselectfields.value == 2) {
        var n = '';
        this.selected.forEach((item) => {
          for(var i=0; i<this.items.length; i++) {
            if (this.items[i].name == item && this.items[i].type != 'd') {
              if(n.length) n += '|';
              n += item;
              break;
            }
          }
        });
        console.log('archive:' + n);
        if(n.length > 0) this.archiveFile(n);
      }
      this.selected = [];
      this.bulkselectfields.value = 0;
    },
    upload(evt) {
      let formData = new FormData();
      formData.append("fwfile", this.currentFile);
      return axios.post(this.$route.fullPath, formData, {
        headers: {
          "Content-Type": "multipart/form-data"
        },
        onUploadProgress: function (progressEvent) {
          this.progress = parseInt(Math.round((progressEvent.loaded / progressEvent.total) * 100));
        }.bind(this)
      });
    },
    selectFile(event) {
      this.progress = 0,
        this.currentFile = event.target.files[0];
    },
    uploadFiles() {
      if (!this.currentFile) {
        this.errorMsg("Please select a file!");
        return;
      }
      this.progress = 0;
      this.upload(this.currentFile)
        .then((response) => {
          this.refreshList(response.data.msg);
          console.log(response.data);
        })
        .catch((e) => {
          this.progress = 0;
          console.log(e);
          this.errorMsg('Could not upload!');
          this.currentFile = undefined;
        });
    },
    dialogResult(response) {
      this.dialog = false;
      this.functionResolve(response);
    },
    createPromise() {
      return new Promise(resolve => {
        this.functionResolve = resolve;
      })
    },
    getLink(name) {
      let base = axios.defaults.baseURL.replace(/\/api.*/, (this.$route.fullPath + '/'));
      return base + name;
    },
    getAll() {
      let id = this.$route.params.id;
      let path = this.$route.fullPath;
      return axios.get(path);
    },
    delete(id) {
      console.log(id);
      if(id && id.indexOf('|')>0) {
        var i = {
          name: id
        };
        return axios.post(this.$route.fullPath + '/delete', i);
      }
      return axios.get(this.$route.fullPath + `/${id}/delete`);
    },
    go(id) {
      console.log(this.$route.fullPath + '/' + id);
      return this.$router.push(this.$route.fullPath + '/' + id);
    },
    downloadFile(id) {
      return axios.get(this.$route.fullPath + `/${id}`);
    },
    archive(id) {
      return axios.get(this.$route.fullPath + `/${id}/archive`);
    },
    retrieveFiles() {
      console.log(this.$route.fullPath);
      this.getAll()
        .then((response) => {
          var d = response.data;
          if (d && d.data && typeof d.data == 'object') {
            d = d.data;
          }
          this.items = d.map(this.getDisplayFile).filter(this.filterDisplayFile);
          // console.log(d);
          this.loadTable = false;
        })
        .catch((e) => {
          console.log(e);
        });
    },
    refreshList(msg) {
      this.snackbar = true;
      this.snackbarText = msg;
      this.retrieveFiles();
    },
    errorMsg(msg) {
      this.snackbar = true;
      this.snackbarText = msg;
      console.log(msg);
    },
    deleteFile(id) {
      this.dialogText = "Delete " + id + "?";
      this.dialog = true;
      this.createPromise().then(result => {
        this.dialog = false;
        this.dialogText = '';
        if (result == true) {
          this.delete(id)
            .then(() => {
              this.refreshList(id + " deleted.");
            })
            .catch((e) => {
              this.errorMsg(id + " delete failed.");
            });
        }
      });
    },
    archiveFile(id) {
      this.archive(id)
        .then(() => {
          this.refreshList(id + " archieved.");
        })
        .catch((e) => {
          this.errorMsg(id + " archieve failed.");
        });
    },
    getDisplayFile(d) {
      return {
        name: d.name,
        size: d.size,
        date: d.date,
        type: d.type,
        actions: null
      };
    },
    filterDisplayFile(d) {
      return d && d.name && !d.name.match(/^(\..|config_)/);
    },
  },
  mounted() {
    //this.retrieveFiles();
  },
  created() {
    if(window.location.origin.includes('esp-logger')) {
      axios.defaults.baseURL = window.location.origin + '/api/v1';
    }
    // watch the params of the route to fetch the data again
    this.$watch(
      () => this.$route.params,
      () => {
        this.retrieveFiles()
      },
      // fetch the data when the view is created and the data is
      // already being observed
      { immediate: true }
    )
    this.retrieveFiles();
  }
}
</script>
<style>
.v-data-table-footer {
  display: none !important;
}

.nowrap-overflow {
  flex-wrap: nowrap;
  overflow-x: auto;
}
</style>
