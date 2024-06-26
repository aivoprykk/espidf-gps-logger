<template>
  <v-app class="myFont">
  <v-layout>
    <v-app-bar app color="primary" prominent>
      <v-app-bar-nav-icon class="hidden-md-and-up" @click.stop="drawer = !drawer"></v-app-bar-nav-icon>
      <v-toolbar-title><v-btn flat to="/">{{ appTitle }}</v-btn></v-toolbar-title>
      <v-spacer></v-spacer>
      <span v-if="!username">
      <v-btn flat class="hidden-sm-and-down" to="/files">Files</v-btn>
      <v-btn flat class="hidden-sm-and-down" to="/config">Config</v-btn>
      <v-btn flat class="hidden-sm-and-down" to="/fwupdate">Firmware</v-btn>
      <v-btn flat class="hidden-sm-and-down" to="/login">Login</v-btn>
      <v-btn flat class="hidden-sm-and-down" @click="requestReatart">Restart</v-btn>
      </span>
      <span v-if="isLoggedIn">
      <v-btn flat class="hidden-sm-and-down">Logout</v-btn>
      </span>
    </v-app-bar>
    <v-navigation-drawer
    app color="primary"
      v-model="drawer"
      temporary
      height="100vh"
      class="px-3 py-3"
    >
      <v-list>
        <v-list-item v-for="(item, i) in items" :key="i" :value="item.title" :to="item.to">
        <template v-slot:prepend>
          <v-icon :icon="item.icon"></v-icon>
        </template>
            <v-list-item-title>{{ item.title }}</v-list-item-title>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>

  <v-main>
    <router-view></router-view>
  </v-main>
  </v-layout>
  </v-app>
</template>

<script>

import { mapGetters } from "vuex";
import axios from './service/api-client.js'
export default {
  name: 'App',
  data() {
    return {
      username:this.$route.params.username,
      appTitle: 'ESP-Logger',
      drawer: false,
      items: [
        { title: 'Home', to: '/', icon: 'home' },
        { title: 'Files', to: '/files', icon: 'folder' },
        { title: 'Config', to: '/config', icon: 'cog-outline' },
        { title: 'Firmware', to: '/fwupdate', icon: 'wrench-cog-outline' },
        { title: 'Login', to: '/login', icon: 'login' }
      ],
    }
  },
  methods: {
    requestReatart() {
      return axios.get('/system/restart');
    }
  },
  computed: {
    ...mapGetters(["isLoggedIn"])
  }
}
</script>

<style>
    iframe#webpack-dev-server-client-overlay{display:none!important}
</style>
