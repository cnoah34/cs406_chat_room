<template>
    <div class="rooms">
        <div class="header">
            <h1 style="color: white;">Your rooms</h1>
            <button @click="show_add_room = !show_add_room" class="add-room-button">{{ show_add_room
                ? 'X' : '+' }}</button>
        </div>
        <div v-if="show_add_room" class="popup-parent">
            <router-link to="/create-room" class="room-link">Create a Room</router-link>
            <router-link to="/join-room" class="room-link">Join a Room</router-link>
        </div>
        <ul v-else>
            <li v-for="room in rooms" :key="room.room_id" @click="selectRoom(room)"
                :class="{ selected: userStore.current_room && room.room_id === userStore.current_room.room_id }">
                {{ room.name }}
            </li>
        </ul>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useUserStore } from '@/store/user'
    import { useApiStore, useWebSocketStore } from '@/store/api'
    import axios from 'axios'

    const router = useRouter()
    const apiStore = useApiStore()
    const webSocketStore = useWebSocketStore()
    const userStore = useUserStore()

    const rooms = ref([])
    const show_add_room = ref(false)

    const getRoom = async (room_id) => {
        try {
            const response = await axios.get(`${apiStore.rest_url}/rooms/${room_id}`, {
                headers: {
                    Authorization: `Bearer ${localStorage.getItem('token')}`
                }
            })

            if (response.status == 200 && response.data) {
                const room = response.data
                room.room_id = room_id  // Query does not include room_id
                rooms.value.push(room) 

                rooms.value.sort((a, b) => {
                    if (a.name < b.name) return -1;
                    if (a.name > b.name) return 1;
                    return 0;
                });
            }
        }
        catch (error) {
            console.error('Failed to fetch room: ', error)
        }
    }

    const selectRoom = (room) => {
        userStore.current_room = room
    }

    onMounted(() => {
        rooms.value = []
        
        // TODO: Query rooms here

        userStore.room_ids.forEach(room_id => {
            getRoom(room_id)
        })
    })
</script>


<style scoped>

.rooms {
    display: flex;
    flex-direction: column;
    height: 80vh;
    background-color: var(--foreground);
}

.header {
    display: flex;
    flex-direction: row;
}

h1 {
    padding-left: 20px;
    margin: 0;
    padding-bottom: 10px;
}

ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    overflow-y: auto;
}

li {
    padding-left: 20px;
    color: white;
    cursor: pointer;
    text-decoration: underline;
    font-size: 14pt;
}

li:hover {
    text-decoration: none;
}

.selected {
    color: var(--vue-green);
    font-size: 18pt;
}

.add-room-button {
    font-size: 20pt;
    color: white;
    min-width: 70px;
    margin: auto;
    background-color: var(--foreground);
    border: var(--vue-green) solid 3px;
    text-align: center;
    align-items: center;
    text-decoration: none;
    cursor: pointer;
}

.add-room-button:hover {
    color: var(--foreground);
    background-color: var(--vue-green)
}

.popup-parent {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
}

.room-link {
    color: white;
    margin-top: 10%;
    min-width: 75%;
    text-align: center;
    border: var(--vue-green) solid 3px;
    font-size: 18pt;
    text-decoration: none;
}

.room-link:hover {
    color: var(--foreground);
    background-color: var(--vue-green)
}

</style>
