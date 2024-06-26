<template>
</template>
<script>
import axios from '../service/api-client.js'
export default {
  name: 'fileList',
  data() {
    return {
      files: [],
      title: '',
      headers: [
        { text: 'Name', align: 'start', sortable: true, value: 'name' },
        { text: 'Date', value: 'date', sortable: true },
        { text: 'Size', value: 'size', sortable: true },
        { text: 'Actions', value: 'actions', sortable: false },
      ],
    };
  },
  methods: {
    getAll() {
      return axios.get('/files');
    },

    delete(id) {
      return axios.delete(`/files/${id}`);
    },

    archive(id) {
      return axios.get(`/files/${id}/archive`);
    },
    retrieveFiles() {
      this.getAll()
        .then((response) => {
          this.files = response.data.map(this.getDisplayFile);
          console.log(response.data);
        })
        .catch((e) => {
          console.log(e);
        });
    },

    refreshList() {
      this.retrieveFiles();
    },

    deleteFile(id) {
      this.delete(id)
        .then(() => {
          this.refreshList();
        })
        .catch((e) => {
          console.log(e);
        });
    },

    archiveFile(id) {
      this.archive(id)
        .then(() => {
          this.refreshList();
        })
        .catch((e) => {
          console.log(e);
        });
    },
    getDisplayFile(file) {
      return {
        name: file.name,
        date: file.date,
        size: file.size,
      };
    },
  },
  mounted() {
    this.retrieveFiles();
  }
}
</script>

<style scoped></style>
