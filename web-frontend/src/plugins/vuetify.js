//import 'vuetify/lib/styles/main.sass'
import "vuetify/styles";
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components';
import * as directives from 'vuetify/directives';
//import * as labsComponents from 'vuetify/labs/VDataTable'

//import "@mdi/font/css/materialdesignicons.css";
import { aliases, mdi } from 'vuetify/iconsets/mdi-svg';
import {
  mdiArchive,
  mdiDelete,
  mdiDownload,
  mdiHome,
  mdiLogin,
  mdiUpload,
} from '@mdi/js'

//import { mdiAccount } from '@mdi/js'
//const aliases = allAliases;
/* const aliases = {
  menu: allAliases.menu,
  close: allAliases.close,
  info: allAliases.info,
  delete: allAliases.delete,
  archive: allAliases.archive,
  download: allAliases.download,
  download: allAliases.download,
  download: allAliases.download,
  account: allAliases.account,
  "arrow-up": allAliases.arrowUp,
  "arrow-down": allAliases.arrowDown,
  home: allAliases.home,
  folder: allAliases.folder,
  "cog-outline": allAliases.cogOutline,
  "menu-down": allAliases.menuDown,
  "menu-up": allAliases.menuUp
}; */
const v = createVuetify({
  theme: {
    defaultTheme: "light",
  },
  VBtn: {
rounded: false,
  },
  components: { ...components },
  directives,
  icons: {
    defaultSet: 'mdi',
    aliases: {
      ...aliases,
      delete: mdiDelete,
      download: mdiDownload,
      archive: mdiArchive,
      home: mdiHome,
      login: mdiLogin,
      upload: mdiUpload,
    },
    sets: { mdi }
  },
  defaultAssets: {
    //font: true,
    //icons: 'mdi'
  }
});

export default v
