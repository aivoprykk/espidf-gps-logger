import { createRouter, createWebHistory } from 'vue-router'
import Home from '../views/Home.vue'
import Files from '../views/Files.vue'
import Config from '../views/Config.vue'
import FirmwareUpdate from '../views/FirmwareUpdate.vue'
import Login from '../views/Login.vue'
const routes = [
  {
    path: '/',
    name: 'Home',
    component: Home
  },
  {
    path: '/files/:id*',
    component: Files,
  },
  {
    path: '/config',
    name: 'Config',
    component: Config
  },
  {
    path: '/fwupdate',
    name: 'Firmware',
    component: FirmwareUpdate
  },
  {
    path: '/login',
    name: 'Login',
    component: Login
  }
]

const router = createRouter({
  history: createWebHistory(''),
  routes
})

export default router
