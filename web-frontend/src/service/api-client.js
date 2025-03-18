import axios from "axios";

export default axios.create({
  baseURL: "http://esp.local/api/v1",
  headers: {
    "Content-type": "application/json"
  },
  withCredentials: false
});
