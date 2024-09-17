<template>
  <v-container>
    <v-card class="mx-auto" tile>
        <v-card-title class="mt-4 text-h6">Firmware update</v-card-title>
    <v-row v-if="currentFile" no-gutters justify="center" align="center">
      <v-col class="col-md-6">
        <v-progress-linear
          v-model="progress"
          color="primary"
          height="20"
          reactive
        >
          <strong>{{ progress }} %</strong>
        </v-progress-linear>
     </v-col>
    </v-row>
    <v-row>
      <v-col class="col-6">
        <v-file-input
          id="file"
          v-model="files"
          color="deep-purple-accent-4"
          show-size
          label="Firmware file"
          @change="selectFile"
          hide-details
              class="pa-0 ma-0"
              single-line
              variant="outlined"
        ></v-file-input>
      </v-col></v-row>
    <v-row>
      <v-col cols="6" offset="5" class="padding-5">
        <v-btn small @click="uploadFiles">
          Upload
          <v-icon right>$upload</v-icon>
        </v-btn>
      </v-col>
    </v-row>

    <v-alert v-if="message" border="left">
      {{ message }}
    </v-alert>

    <v-row >
      <v-list>
        <v-list-subheader class="ma-auto"> <span class="grey--text">Current Firmware version: {{ firmwareversion }}</span></v-list-subheader>
      </v-list>
  </v-row>
  </v-card>
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
  name: 'firmwareUpdate',
  data() {
    return {
      currentFile: undefined,
      progress: 0,
      message: '',
      firmwareversion: '',
      files: [],
      snackbar: false,
      snackbarText: '',
      snackbarTimeout: 1000
    }
  },
  methods: {
    upload(evt) {
      let formData = new FormData();
      formData.append("fwfile", this.currentFile);
      return axios.post("/fw/update", formData, {
        headers: {
          "Content-Type": "multipart/form-data"
        },
        onUploadProgress: function (progressEvent) {
          this.progress = parseInt(Math.round((progressEvent.loaded / progressEvent.total) * 100));
        }.bind(this)
      });
    },
    getFirmwareVersion() {
      console.log(this.$route.fullPath);
      axios.get("/fw/version").then(response => {
        var d = response.data;
        if (d && d.data && typeof d.data == 'object') {
          d = d.data;
        }
        this.firmwareversion = d.version;
        console.log(this.firmwareversion);
      }).catch((e) => {
        console.log(e);
      });
    },
    selectFile(event) {
      this.progress = 0;
      this.currentFile = event.target.files[0];
    },
    refresh(msg) {
      this.snackbar = true;
      this.snackbarText = msg;
      this.getFirmwareVersion();
    },
    uploadFiles() {
      if (!this.currentFile) {
        this.message = "Please select a file!";
        return;
      }
      this.progress = 0;
      this.upload(this.currentFile)
        .then((response) => {
          this.message = response.data.msg;
          this.refresh(response.data.msg);
        })
        .catch(() => {
          this.progress = 0;
          this.message = "Could not upload the file!";
          this.currentFile = undefined;
        });
    }
  },
  mounted() {
    axios.defaults.baseURL = window.location.origin + '/api/v1';
    this.getFirmwareVersion();
  }
}
</script>
<style>
.padding-5 {
  padding-left: 0;
}

.cardpadding {
  min-height: 40vh;
}

.v-file-input {
  min-height: 54px !important;
}
</style>
