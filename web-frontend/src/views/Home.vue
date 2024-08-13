<template>
  <v-container>
    <v-row align="center" justify="center" style="height:100vh" dense>
      <v-col cols="12" sm="6" offset-sm="3" class="grey lighten-2 fill-height d-flex flex-column justify-center align-center">
        <v-card tile>
          <v-img :src="require('../assets/esp-gps-logo-v2.png')" contain height="200"></v-img>
          <v-card-title primary-title>
            <div class="ma-auto" density="compact">
              <v-row dense> <v-col cols=6><span class="grey--text">ESP Version:</span></v-col> <v-col>{{ version }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">ESP cores:</span></v-col> <v-col>{{ cores }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">SW Version:</span></v-col> <v-col> {{ fwver }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">Hostname:</span></v-col> <v-col> {{ hostname }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">AP SSID:</span></v-col> <v-col> {{ ap_ssid }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">AP IP:</span></v-col> <v-col> {{ ap_ip }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">STA SSID:</span></v-col> <v-col> {{ sta_ssid }}</v-col></v-row>
              <v-row dense> <v-col cols=6><span class="grey--text">STA IP:</span></v-col> <v-col> {{ sta_ip }}</v-col></v-row>
            </div>
          </v-card-title>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>

import axios from '../service/api-client.js'
export default {
  data() {
    return {
      version: null,
      cores: null,
      fwver: null,
      hostname: null,
      ap_ssid: null,
      ap_ip: null,
      sta_ssid: null,
      sta_ip: null,
    }
  },
  mounted() {
    if(window.location.origin.includes('esp-logger')) {
      axios.defaults.baseURL = window.location.origin + '/api/v1';
    }
    axios.get('/system/info')
      .then(response => {
        var d = response.data;
        if (d && d.data && typeof d.data == 'object') {
          d = d.data;
        }
        this.version = d ? d.version : ''
        this.cores = d ? d.cores : ''
        this.fwver = d ? d.fwversion : '',
        this.hostname = d ? d.hostname : ''
        this.ap_ssid = d ? d.ap_ssid : ''
        this.ap_ip = d ? d.ap_address : ''
        this.sta_ssid = d ? d.sta_ssid : ''
        this.sta_ip = d ? d.sta_address : ''
      })
      .catch(error => {
        console.log(error)
      })
  }
}
</script>
