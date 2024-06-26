<template>
  <v-layout>
    <v-app-bar app color="primary" prominent>
      <v-app-bar-nav-icon variant="text" @click.stop="drawer = !drawer"></v-app-bar-nav-icon>

      <v-toolbar-title>{{ appTitle }}</v-toolbar-title>
      <v-spacer></v-spacer>
      <v-btn variant="text">Files</v-btn>
      <v-btn variant="text">Config</v-btn>
      <v-btn variant="text">Firmware</v-btn>
      <v-spacer></v-spacer>
      <v-btn variant="text">Login</v-btn>
    </v-app-bar>
    <v-navigation-drawer
      v-model="drawer"
      temporary
    >
      <v-list>
        <v-list-item v-for="(item, i) in items" :key="i" :value="item.title">

            <v-list-item-title>{{ item.title }}</v-list-item-title>

        </v-list-item>
      </v-list>
    </v-navigation-drawer>

  </v-layout>
</template>

<script>
export default {
  name: 'AppNavigation',
  data() {
    return {
      appTitle: 'ESP-Logger',
      drawer: false,
      items: [
        { title: 'Files', to: '/files' },
        { title: 'Config', to: '/config' },
        { title: 'Firmware', to: '/fwupdate' },
        { title: 'Login', to: '' }
      ],
    }
  }
}
</script>

<style scoped></style>
