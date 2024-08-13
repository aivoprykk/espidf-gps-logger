<template>
  <v-container>
    <v-row align="center" justify="center" style="height:100vh" dense>
      <v-col cols="12" sm="6" offset-sm="3" class="grey lighten-2 fill-height d-flex flex-column justify-center align-center">
  <v-card class="mx-auto mt-12" max-width="500">
    <v-img :src="require('../assets/esp-gps-logo-v2.png')" contain height="100"></v-img>
        <v-card-text>
            <v-form>
                <v-text-field
                v-model="username"
                    label="Username"
                    :rules="[v => !!v || 'Username is required']"
                    required
                    single-line
                    variant="outlined"
                ></v-text-field>
                <v-text-field
                v-model="password"
                    label="Password"
                    :rules="[v => !!v || 'Password is required']"
                    type="password"
                    required
                    single-line
                    variant="outlined"
                ></v-text-field>
            </v-form>
        </v-card-text>
        <v-card-actions>
            <v-btn block size="large" variant="outlined" color="primary" @click="login()">
                Login
            </v-btn>
        </v-card-actions>
    </v-card>
    </v-col></v-row>
    </v-container>
</template>

<script>

import axios from '../service/api-client.js'
import { mapMutations } from "vuex";
export default {
  data: () => {
    return {
      username: "",
      password: "",
      token: ""
    };
  },
  mounted() {
    if(window.location.origin.includes('esp-logger')) {
      axios.defaults.baseURL = window.location.origin + '/api/v1';
    }
  },
  methods: {
    ...mapMutations(["setUser", "setToken"]),
    login(e) {
      //e.preventDefault();
      axios.get('/login', {
          username: this.username,
          password: this.password
      },{
        headers: {
          'Content-Type': 'application/json'
        }
      })
      .then(response => {
        var d = response.data;
        if(d && d.data && typeof d.data == 'object'){
          d = d.data;
        }
        if(d.username && d.token) {
          this.setUser(d.user);
          this.setToken(d.token);
          this.$router.push("/");
        }
        else {
          console.log(d);
        }
      })
      .catch(error => {
        console.log(error)
      })
    }
  }
};
</script>

<style>
.v-field__input {
  min-height: 0 !important;
}
.v-card-actions {
  padding: 0 15px 20px 15px !important;
}
.v-card-text {
  padding-bottom: 0 !important;
}
</style>
