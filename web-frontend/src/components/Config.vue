<template>
  <v-row align="center" class="list px-3 mx-auto">
    <v-col cols="12" sm="12">
      <v-card class="mx-auto" tile>
        <v-card-title>config</v-card-title>
        <v-data-table
          :headers="headers"
          :items="config"
          disable-pagination
          :hide-default-footer="true"
        >
        </v-data-table>
      </v-card>
    </v-col>
  </v-row>
</template>
<script>
import axios from '../service/api-client.js'
export default {
  name: "configs",
  data() {
    return {
      config: [],
      title: '',
      headers: [
        { text: 'Name', align: 'start', sortable: true, value: 'name' },
        { text: 'Value', value: 'value', sortable: true },
        { text: 'Description', value: 'desc', sortable: false },
        { text: 'Type', value: 'type', sortable: false },
      ],
    };
  },
  methods: {
    getAll() {
      return axios.get('/config');
    },
    update(id) {
      return axios.put(`/config/${id}`);
    },
    retrieveConfig() {
      this.getAll()
        .then((response) => {
          this.config = response.data.map(this.getDisplayConf);
          console.log(response.data);
        })
        .catch((e) => {
          console.log(e);
        });
    },

    refreshList() {
      this.retrieveconfig();
    },
    updateConf(id) {
      this.update(id)
        .then(() => {
          this.refreshList();
        })
        .catch((e) => {
          console.log(e);
        });
    },
    getDisplayConf(conf) {
      return {
        name: conf.name,
        value: conf.value,
        desc: conf.desc,
        type: conf.type,
      };
    },
  },
  mounted() {
    this.retrieveConfig();
  },
};
</script>

<style scoped></style>
