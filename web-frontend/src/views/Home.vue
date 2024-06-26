<template>
  <v-container>
    <v-row align="center" justify="center" style="height:100vh" dense>
      <v-col cols="12" sm="6" offset-sm="3" class="grey lighten-2 fill-height d-flex flex-column justify-center align-center">
        <v-card tile>
          <v-img :src="require('../assets/esp-gps-logo-v2.png')" contain height="200"></v-img>
          <v-card-title primary-title>
            <div class="ma-auto">
              <span class="grey--text">ESP Version: {{ version }}</span>
              <br>
              <span class="grey--text">ESP cores: {{ cores }}</span>
              <br>
              <span class="grey--text">SW Version: {{ fwver }}</span>
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
      fwver: null
    }
  },
  mounted() {
    axios
      .get('/system/info')
      .then(response => {
        var d = response.data;
        if (d && d.data && typeof d.data == 'object') {
          d = d.data;
        }
        this.version = d ? d.version : ''
        this.cores = d ? d.cores : ''
        this.fwver = d ? d.fwversion : ''
      })
      .catch(error => {
        console.log(error)
      })
  }
}
</script>
